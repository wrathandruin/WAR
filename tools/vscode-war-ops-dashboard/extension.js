const vscode = require("vscode");
const path = require("path");
const fs = require("fs");
const crypto = require("crypto");
const { execFile, execFileSync, spawn } = require("child_process");

const {
  ACTIONS,
  SECTIONS,
  STATUS_DEFINITIONS,
  TOP_ACTION_IDS
} = require("./registry");

const VIEW_TYPE = "warOps.panel";
const POLL_INTERVAL_MS = 30000;
const ACTION_COMMAND_PREFIX = "warOps.action.";
const ACTION_COMMAND_EXCLUSIONS = new Set(["refreshDashboard", "openOutput"]);
const REQUIRED_STAGE_FILES = [
  "WAR.exe",
  "WARServer.exe",
  "launch_headless_host_win64.bat",
  "launch_local_demo_win64.bat",
  "launch_local_client_against_host_win64.bat",
  "smoke_test_headless_host_win64.bat",
  "smoke_test_local_demo_win64.bat"
];
const STAGE_MANIFEST_FILES = [
  "internal_alpha_manifest.txt",
  "beta_release_candidate_manifest.txt",
  "market_candidate_manifest.txt",
  "demo_manifest.txt"
];

function toActionCommandId(actionId) {
  return `${ACTION_COMMAND_PREFIX}${actionId}`;
}

class WarOperationsDashboardProvider {
  constructor() {
    this.output = vscode.window.createOutputChannel("WAR Ops: Dashboard");
    this.statusBarItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
    this.view = null;
    this.timer = null;
    this.refreshInFlight = false;
    this.actionInFlight = false;
    this.state = this.createInitialState();
  }

  createInitialState() {
    const statuses = {};
    for (const definition of STATUS_DEFINITIONS) {
      statuses[definition.id] = this.makeStatus("unknown", definition.label, "Waiting for first refresh");
    }

    return {
      busy: "",
      lastRefresh: "Never",
      lastAction: null,
      paths: null,
      statuses
    };
  }

  initialize() {
    this.statusBarItem.command = "warOps.openDashboard";
    this.statusBarItem.name = "WAR Ops: Dashboard";
    this.statusBarItem.text = "$(dashboard) WAR Ops";
    this.statusBarItem.tooltip = "Open the WAR Ops dashboard.";
    this.statusBarItem.show();

    this.startPolling();
    void this.refreshStatuses(false);
  }

  dispose() {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }

    this.statusBarItem.dispose();
    this.output.dispose();
  }

  resolveWebviewView(webviewView) {
    this.view = webviewView;
    webviewView.webview.options = {
      enableScripts: true,
      localResourceRoots: [vscode.Uri.file(path.join(__dirname, "media"))]
    };
    webviewView.webview.html = this.renderHtml(webviewView.webview);

    webviewView.webview.onDidReceiveMessage(async message => {
      try {
        switch (message?.type) {
          case "ready":
            this.postState();
            break;

          case "refresh":
            await this.refreshStatuses(true);
            break;

          case "action":
            if (typeof message.actionId === "string") {
              await this.handleAction(message.actionId);
            }
            break;

          default:
            break;
        }
      } catch (error) {
        const summary = this.formatError(error);
        this.output.appendLine(`[webview-message-error] ${summary}`);
        void vscode.window.showErrorMessage(`WAR Ops error: ${summary}`);
      }
    });

    this.postState();
  }

  async focusSidebar() {
    const candidates = ["workbench.view.extension.warOps", "workbench.view.warOps"];

    for (const command of candidates) {
      try {
        await vscode.commands.executeCommand(command);
        return;
      } catch (error) {
        this.output.appendLine(`[focus-sidebar-error] ${command}: ${this.formatError(error)}`);
      }
    }
  }

  startPolling() {
    if (this.timer) {
      clearInterval(this.timer);
    }

    this.timer = setInterval(() => {
      void this.refreshStatuses(false);
    }, POLL_INTERVAL_MS);
  }

  async handleAction(actionId) {
    const action = ACTIONS[actionId];
    if (!action) {
      void vscode.window.showWarningMessage(`Unknown WAR Ops action: ${actionId}`);
      return;
    }

    if (this.actionInFlight && !action.background) {
      void vscode.window.showInformationMessage("Another WAR Ops action is already running.");
      return;
    }

    if (action.kind === "utility") {
      await this.runUtility(action);
      return;
    }

    if (action.kind === "openFile") {
      await this.openActionFile(action);
      return;
    }

    const inputValue = await this.collectActionInput(action);
    if (action.prompt && !inputValue) {
      return;
    }

    const confirmed = await this.confirmAction(this.resolveConfirmMessage(action, inputValue));
    if (!confirmed) {
      return;
    }

    if (action.terminal) {
      this.openTerminal(action.label, action, inputValue);
      this.recordActionResult(action.label, true, `Opened terminal for ${action.label}.`);
      return;
    }

    if (action.background) {
      await this.runBackgroundAction(action, inputValue);
      return;
    }

    await this.runAction(action, inputValue);
  }

  async runUtility(action) {
    switch (action.utility) {
      case "refresh":
        await this.refreshStatuses(true);
        return;

      case "output":
        this.output.show(true);
        return;

      default:
        void vscode.window.showWarningMessage(`Unknown WAR Ops utility: ${action.utility}`);
    }
  }

  async openActionFile(action) {
    const absolutePath = this.resolveActionFilePath(action);
    const document = await vscode.workspace.openTextDocument(absolutePath);
    await vscode.window.showTextDocument(document, { preview: false });
  }

  async runAction(action, inputValue) {
    this.actionInFlight = true;
    this.state.busy = action.label;
    this.postState();

    try {
      const result = await vscode.window.withProgress(
        {
          location: vscode.ProgressLocation.Notification,
          title: `WAR Ops - ${action.label}`,
          cancellable: false
        },
        async () => this.execAction(action, inputValue)
      );

      const output = this.combineOutput(result.stdout, result.stderr);
      this.output.appendLine(`\n=== ${action.label} ===`);
      this.output.appendLine(output || "(no output)");
      this.recordActionResult(action.label, true, output || "Completed without output.");
    } catch (error) {
      const message = this.formatError(error);
      this.output.appendLine(`\n=== ${action.label} FAILED ===`);
      this.output.appendLine(message);
      this.recordActionResult(action.label, false, message);
      void vscode.window.showErrorMessage(`${action.label} failed. See the WAR Ops output for details.`);
    } finally {
      this.actionInFlight = false;
      this.state.busy = "";
      this.postState();
      await this.refreshStatuses(false);
    }
  }

  async runBackgroundAction(action, inputValue) {
    const invocation = this.buildActionInvocation(action, inputValue);

    try {
      await vscode.window.withProgress(
        {
          location: vscode.ProgressLocation.Notification,
          title: `WAR Ops - ${action.label}`,
          cancellable: false
        },
        async () => new Promise((resolve, reject) => {
          const child = spawn(invocation.command, invocation.args, {
            cwd: invocation.cwd,
            detached: true,
            stdio: "ignore"
          });

          child.once("error", reject);
          child.unref();
          resolve();
        })
      );

      this.recordActionResult(action.label, true, `Launched ${action.label} in the background.`);
      void vscode.window.setStatusBarMessage(`${action.label} launched`, 2500);
    } catch (error) {
      const message = this.formatError(error);
      this.output.appendLine(`\n=== ${action.label} FAILED ===`);
      this.output.appendLine(message);
      this.recordActionResult(action.label, false, message);
      void vscode.window.showErrorMessage(`${action.label} failed. See the WAR Ops output for details.`);
    }
  }

  async refreshStatuses(userInitiated) {
    if (this.refreshInFlight) {
      return;
    }

    this.refreshInFlight = true;

    try {
      const paths = this.resolveWorkspacePaths();
      this.state.paths = {
        workspaceRoot: paths.workspaceRoot,
        repoRoot: paths.repoRoot,
        stageRoot: paths.stageRoot || "No staged package found",
        runtimeRoot: paths.runtimeRoot || "No runtime root found"
      };

      const statuses = {};
      for (const definition of STATUS_DEFINITIONS) {
        statuses[definition.id] = await this.collectStatus(definition, paths);
      }

      this.state.statuses = statuses;
      this.state.lastRefresh = new Date().toLocaleTimeString();
      this.updateStatusBar();
      this.postState();

      if (userInitiated) {
        void vscode.window.setStatusBarMessage("WAR Ops status refreshed", 2500);
      }
    } catch (error) {
      const message = this.formatError(error);
      this.output.appendLine(`[refresh-error] ${message}`);
      this.updateStatusBar(message);
      this.postState();

      if (userInitiated) {
        void vscode.window.showErrorMessage(`WAR Ops refresh failed: ${message}`);
      }
    } finally {
      this.refreshInFlight = false;
    }
  }

  async collectStatus(definition, paths) {
    switch (definition.kind) {
      case "stagePackage":
        return this.collectStagePackageStatus(definition, paths);

      case "report":
        return this.collectReportStatus(definition, paths);

      case "bundleReport":
        return this.collectBundleReportStatus(definition, paths);

      case "runtimeFiles":
        return this.collectRuntimeFilesStatus(definition, paths);

      default:
        return this.makeStatus("unknown", definition.label, "Unsupported status definition");
    }
  }

  collectStagePackageStatus(definition, paths) {
    if (!paths.stageRoot) {
      return this.makeStatus("warning", definition.label, "No staged package found");
    }

    const missingPaths = REQUIRED_STAGE_FILES.filter(relativePath => {
      return !fs.existsSync(path.join(paths.stageRoot, relativePath));
    });
    if (!this.findStageManifestFile(paths.stageRoot)) {
      missingPaths.push("lane manifest");
    }

    if (missingPaths.length > 0) {
      return this.makeStatus(
        "warning",
        definition.label,
        `${paths.stageLabel} missing ${missingPaths[0]}`
      );
    }

    return this.makeStatus("healthy", definition.label, `${paths.stageLabel} ready`);
  }

  collectReportStatus(definition, paths) {
    const base = this.resolveStatusBase(definition, paths);
    if (!base.path) {
      return this.makeStatus("warning", definition.label, `No ${base.label} resolved`);
    }

    const reportPath = path.join(base.path, definition.relativePath);
    if (!fs.existsSync(reportPath)) {
      return this.makeStatus(
        "warning",
        definition.label,
        `${base.displayLabel}: no report yet`
      );
    }

    let content = "";
    try {
      content = fs.readFileSync(reportPath, "utf8");
    } catch (error) {
      return this.makeStatus("error", definition.label, this.formatError(error));
    }

    if (/Result:\s*PASS/i.test(content)) {
      return this.makeStatus("healthy", definition.label, `${base.displayLabel}: PASS`);
    }

    if (/Result:\s*FAIL/i.test(content)) {
      return this.makeStatus("error", definition.label, `${base.displayLabel}: FAIL`);
    }

    return this.makeStatus(
      "warning",
      definition.label,
      this.firstUsefulLine(content) || `${base.displayLabel}: report present`
    );
  }

  collectBundleReportStatus(definition, paths) {
    const base = this.resolveStatusBase(definition, paths);
    if (!base.path) {
      return this.makeStatus("warning", definition.label, `No ${base.label} resolved`);
    }

    const total = definition.relativePaths.length;
    let passCount = 0;
    let failCount = 0;
    let missingCount = 0;

    for (const relativePath of definition.relativePaths) {
      const reportPath = path.join(base.path, relativePath);
      if (!fs.existsSync(reportPath)) {
        missingCount += 1;
        continue;
      }

      let content = "";
      try {
        content = fs.readFileSync(reportPath, "utf8");
      } catch (error) {
        return this.makeStatus("error", definition.label, this.formatError(error));
      }

      if (/Result:\s*FAIL/i.test(content)) {
        failCount += 1;
        continue;
      }

      if (/Result:\s*PASS/i.test(content)) {
        passCount += 1;
      }
    }

    if (failCount > 0) {
      return this.makeStatus("error", definition.label, `${failCount}/${total} checks failed`);
    }

    if (passCount === total) {
      return this.makeStatus("healthy", definition.label, `${passCount}/${total} checks passed`);
    }

    if (missingCount === total) {
      return this.makeStatus("warning", definition.label, `${base.displayLabel}: no reports yet`);
    }

    return this.makeStatus(
      "warning",
      definition.label,
      `${passCount}/${total} checks passed${missingCount ? `, ${missingCount} missing` : ""}`
    );
  }

  collectRuntimeFilesStatus(definition, paths) {
    if (!paths.runtimeRoot) {
      return this.makeStatus("warning", definition.label, "No runtime root resolved");
    }

    const presentCount = definition.relativePaths.filter(relativePath => {
      return fs.existsSync(path.join(paths.runtimeRoot, relativePath));
    }).length;

    if (presentCount === definition.relativePaths.length) {
      return this.makeStatus(
        "healthy",
        definition.label,
        `${presentCount}/${definition.relativePaths.length} artifacts present`
      );
    }

    if (presentCount === 0) {
      return this.makeStatus("warning", definition.label, "Runtime artifacts not generated yet");
    }

    return this.makeStatus(
      "warning",
      definition.label,
      `${presentCount}/${definition.relativePaths.length} artifacts present`
    );
  }

  resolveStatusBase(definition, paths) {
    if (definition.root === "stage") {
      return {
        path: paths.stageRoot,
        label: "staged package",
        displayLabel: paths.stageLabel || "staged package"
      };
    }

    if (definition.root === "repo") {
      return {
        path: paths.repoRoot,
        label: "repo root",
        displayLabel: "repo"
      };
    }

    return {
      path: paths.runtimeRoot,
      label: "runtime root",
      displayLabel: paths.runtimeLabel
    };
  }

  makeStatus(tone, label, detail) {
    return { tone, label, detail };
  }

  updateStatusBar(errorMessage = "") {
    const stage = this.state.statuses.stagePackage;
    const iconMap = {
      healthy: "$(pass-filled)",
      warning: "$(warning)",
      error: "$(error)",
      unknown: "$(question)"
    };
    const icon = iconMap[stage?.tone] || iconMap.unknown;

    this.statusBarItem.text = `${icon} WAR Ops`;
    this.statusBarItem.tooltip = this.buildStatusTooltip(errorMessage);
    this.statusBarItem.backgroundColor = undefined;

    if (stage?.tone === "error") {
      this.statusBarItem.backgroundColor = new vscode.ThemeColor("statusBarItem.errorBackground");
    } else if (stage?.tone === "warning") {
      this.statusBarItem.backgroundColor = new vscode.ThemeColor("statusBarItem.warningBackground");
    }
  }

  buildStatusTooltip(errorMessage) {
    const lines = ["WAR Ops dashboard", ""];

    for (const definition of STATUS_DEFINITIONS) {
      const status = this.state.statuses[definition.id];
      lines.push(`${status.label}: ${status.detail}`);
    }

    lines.push("", `Last refresh: ${this.state.lastRefresh}`);

    if (this.state.paths) {
      lines.push("", `Workspace: ${this.state.paths.workspaceRoot}`);
      lines.push(`Repo: ${this.state.paths.repoRoot}`);
      lines.push(`Stage: ${this.state.paths.stageRoot}`);
      lines.push(`Runtime: ${this.state.paths.runtimeRoot}`);
    }

    if (errorMessage) {
      lines.push("", `Last error: ${errorMessage}`);
    }

    return lines.join("\n");
  }

  postState() {
    if (!this.view) {
      return;
    }

    this.view.webview.postMessage({
      type: "state",
      state: this.buildViewState()
    });
  }

  buildViewState() {
    const topActions = TOP_ACTION_IDS.map(id => this.summarizeAction(ACTIONS[id]));

    const sections = SECTIONS.map(section => ({
      id: section.id,
      title: section.title,
      description: section.description,
      groups: section.groups.map(group => ({
        title: group.title,
        actions: group.actionIds.map(actionId => this.summarizeAction(ACTIONS[actionId])).filter(Boolean)
      }))
    }));

    const statuses = STATUS_DEFINITIONS.map(definition => this.state.statuses[definition.id]);

    return {
      busy: this.state.busy,
      lastRefresh: this.state.lastRefresh,
      lastAction: this.state.lastAction,
      paths: this.state.paths,
      statuses,
      topActions,
      sections
    };
  }

  summarizeAction(action) {
    if (!action) {
      return null;
    }

    return {
      id: action.id,
      label: action.label,
      description: action.description,
      tone: action.tone || "normal"
    };
  }

  async collectActionInput(action) {
    if (!action.prompt) {
      return "";
    }

    const value = await vscode.window.showInputBox({
      title: action.prompt.title,
      prompt: action.prompt.prompt,
      placeHolder: action.prompt.placeHolder
    });

    if (!value) {
      return "";
    }

    if (typeof action.prompt.validate === "function" && !action.prompt.validate(value)) {
      void vscode.window.showErrorMessage("Input contains unsupported characters.");
      return "";
    }

    return value;
  }

  resolveConfirmMessage(action, inputValue) {
    if (!action.confirmMessage) {
      return "";
    }

    if (typeof action.confirmMessage === "function") {
      return action.confirmMessage(inputValue);
    }

    return action.confirmMessage;
  }

  async confirmAction(confirmMessage) {
    if (!confirmMessage) {
      return true;
    }

    const selection = await vscode.window.showWarningMessage(confirmMessage, { modal: true }, "Continue");
    return selection === "Continue";
  }

  resolveActionFilePath(action) {
    const paths = this.resolveWorkspacePaths();
    return path.join(action.root === "stage" ? paths.stageRoot : paths.repoRoot, action.relativePath);
  }

  getWorkspaceRoot() {
    const folder = vscode.workspace.workspaceFolders?.[0];
    if (!folder) {
      throw new Error("Open the WAR workspace before using WAR Ops.");
    }

    return folder.uri.fsPath;
  }

  isRepoRoot(candidate) {
    return (
      fs.existsSync(path.join(candidate, "WAR.sln")) &&
      fs.existsSync(path.join(candidate, "scripts")) &&
      fs.existsSync(path.join(candidate, "desktop")) &&
      fs.existsSync(path.join(candidate, "server")) &&
      fs.existsSync(path.join(candidate, "shared"))
    );
  }

  findRepoRoot(startPath) {
    let current = path.resolve(startPath);
    const visited = new Set();

    while (!visited.has(current)) {
      visited.add(current);

      if (this.isRepoRoot(current)) {
        return current;
      }

      const parent = path.dirname(current);
      if (parent === current) {
        break;
      }

      current = parent;
    }

    throw new Error("Could not locate the WAR repository layout. Open the repository root or a folder inside it.");
  }

  parseStageName(name) {
    const match = /^WAR_M(\d+)_(Debug|Release)$/i.exec(name);
    if (!match) {
      return null;
    }

    return {
      name,
      milestone: Number.parseInt(match[1], 10),
      config: match[2],
      configRank: match[2].toLowerCase() === "release" ? 2 : 1
    };
  }

  findStageManifestFile(stageRoot) {
    return STAGE_MANIFEST_FILES.find(relativePath => {
      return fs.existsSync(path.join(stageRoot, relativePath));
    }) || "";
  }

  isUsableStageRoot(stageRoot) {
    return REQUIRED_STAGE_FILES.every(relativePath => {
      return fs.existsSync(path.join(stageRoot, relativePath));
    }) && Boolean(this.findStageManifestFile(stageRoot));
  }

  findLatestStageInfo(repoRoot) {
    const laneRoots = [
      {
        basePath: path.join(repoRoot, "out", "market_candidate"),
        laneLabel: "market candidate",
        laneRank: 4
      },
      {
        basePath: path.join(repoRoot, "out", "beta_candidate"),
        laneLabel: "beta candidate",
        laneRank: 3
      },
      {
        basePath: path.join(repoRoot, "out", "internal_alpha"),
        laneLabel: "internal alpha",
        laneRank: 2
      },
      {
        basePath: path.join(repoRoot, "out", "local_demo"),
        laneLabel: "local demo",
        laneRank: 1
      }
    ];

    const candidates = laneRoots
      .filter(lane => fs.existsSync(lane.basePath))
      .flatMap(lane => fs.readdirSync(lane.basePath, { withFileTypes: true })
        .filter(entry => entry.isDirectory())
        .map(entry => this.parseStageName(entry.name))
        .filter(Boolean)
        .map(parsed => ({
          ...parsed,
          laneLabel: lane.laneLabel,
          laneRank: lane.laneRank,
          basePath: lane.basePath
        })))
      .sort((left, right) => {
        if (right.milestone !== left.milestone) {
          return right.milestone - left.milestone;
        }

        if (right.configRank !== left.configRank) {
          return right.configRank - left.configRank;
        }

        if (right.laneRank !== left.laneRank) {
          return right.laneRank - left.laneRank;
        }

        return left.name.localeCompare(right.name);
      });

    if (candidates.length === 0) {
      return null;
    }

    const best = candidates.find(candidate => {
      return this.isUsableStageRoot(path.join(candidate.basePath, candidate.name));
    });
    if (!best) {
      return null;
    }

    const stageRoot = path.join(best.basePath, best.name);

    return {
      stageRoot,
      stageLabel: `${best.name} (${best.laneLabel})`,
      runtimeRoot: path.join(stageRoot, "runtime"),
      runtimeLabel: `${best.name}/runtime`
    };
  }

  resolveWorkspacePaths() {
    const workspaceRoot = this.getWorkspaceRoot();
    const repoRoot = this.findRepoRoot(workspaceRoot);
    const stageInfo = this.findLatestStageInfo(repoRoot);

    const runtimeRoot = stageInfo?.runtimeRoot || path.join(repoRoot, "Runtime");
    const runtimeLabel = stageInfo?.runtimeLabel || "Runtime";

    return {
      workspaceRoot,
      repoRoot,
      stageRoot: stageInfo?.stageRoot || "",
      stageLabel: stageInfo?.stageLabel || "",
      runtimeRoot,
      runtimeLabel
    };
  }

  execAction(action, inputValue) {
    const invocation = this.buildExecInvocation(action, inputValue);

    return new Promise((resolve, reject) => {
      execFile(
        invocation.command,
        invocation.args,
        {
          cwd: invocation.cwd,
          maxBuffer: 16 * 1024 * 1024
        },
        (error, stdout, stderr) => {
          if (error) {
            const wrapped = new Error(this.combineOutput(stdout, stderr) || error.message);
            wrapped.code = error.code;
            reject(wrapped);
            return;
          }

          resolve({ stdout, stderr });
        }
      );
    });
  }

  buildExecInvocation(action, inputValue) {
    const scriptInvocation = this.resolveScriptInvocation(action, inputValue);

    if (this.isBatchScript(scriptInvocation.scriptPath)) {
      return {
        command: "cmd.exe",
        args: [
          "/d",
          "/s",
          "/c",
          this.buildCmdBatchCall(scriptInvocation.scriptPathWindows, scriptInvocation.args)
        ],
        cwd: scriptInvocation.cwd
      };
    }

    return {
      command: this.getPowerShellExecutable(),
      args: [
        "-NoLogo",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        this.buildPowerShellCall(scriptInvocation.scriptPathWindows, scriptInvocation.args)
      ],
      cwd: scriptInvocation.cwd
    };
  }

  buildActionInvocation(action, inputValue) {
    const scriptInvocation = this.resolveScriptInvocation(action, inputValue);

    return {
      command: this.getPowerShellExecutable(),
      args: [
        "-NoLogo",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-Command",
        this.buildPowerShellCall(scriptInvocation.scriptPathWindows, scriptInvocation.args)
      ],
      cwd: scriptInvocation.cwd
    };
  }

  resolveScriptInvocation(action, inputValue) {
    if (action.kind !== "script") {
      throw new Error(`Unsupported runnable action kind: ${action.kind}`);
    }

    const paths = this.resolveWorkspacePaths();
    const baseRoot = action.root === "stage" ? paths.stageRoot : paths.repoRoot;
    if (action.root === "stage" && !baseRoot) {
      throw new Error("No staged package is available yet. Run Package Release or Package Debug first.");
    }

    const scriptPath = path.join(baseRoot, action.relativePath);
    const scriptPathWindows = this.toWindowsPath(scriptPath);
    const args = Array.isArray(action.args)
      ? action.args.map(arg => this.resolveActionArgument(arg, paths))
      : [];

    if (!fs.existsSync(scriptPath)) {
      throw new Error(`Missing staged script: ${scriptPath}. Rebuild the package from the dashboard first.`);
    }

    if (inputValue) {
      args.push(inputValue);
    }

    return {
      scriptPath,
      scriptPathWindows,
      args,
      cwd: baseRoot
    };
  }

  resolveActionArgument(value, paths) {
    if (value === "{repoRoot}") {
      return this.toWindowsPath(paths.repoRoot);
    }

    if (value === "{stageRoot}") {
      return paths.stageRoot ? this.toWindowsPath(paths.stageRoot) : "";
    }

    return value;
  }

  openTerminal(name, action, inputValue) {
    const invocation = this.buildActionInvocation(action, inputValue);
    const paths = this.resolveWorkspacePaths();
    const terminal = vscode.window.createTerminal({
      name: `WAR: ${name}`,
      cwd: paths.repoRoot
    });
    terminal.show(true);
    const commandLine = process.platform === "win32"
      ? `${invocation.command} ${invocation.args.map(arg => this.terminalQuote(arg)).join(" ")}`
      : `${invocation.command} ${invocation.args.map(arg => this.shellQuote(arg)).join(" ")}`;
    terminal.sendText(commandLine, true);
  }

  getPowerShellExecutable() {
    return "powershell.exe";
  }

  buildPowerShellCall(scriptPathWindows, args) {
    const quotedArgs = args.map(value => this.powerShellQuote(value)).join(" ");
    return `& ${this.powerShellQuote(scriptPathWindows)}${quotedArgs ? ` ${quotedArgs}` : ""}`;
  }

  buildCmdBatchCall(scriptPathWindows, args) {
    const quotedArgs = args.map(value => this.cmdQuote(value)).join(" ");
    return `call ${this.cmdQuote(scriptPathWindows)}${quotedArgs ? ` ${quotedArgs}` : ""}`;
  }

  isBatchScript(scriptPath) {
    const extension = path.extname(scriptPath).toLowerCase();
    return extension === ".bat" || extension === ".cmd";
  }

  toWindowsPath(fsPath) {
    if (process.platform === "win32") {
      return path.win32.normalize(fsPath);
    }

    try {
      return execFileSync("wslpath", ["-w", fsPath], { encoding: "utf8" }).trim();
    } catch (error) {
      this.output.appendLine(`[path-conversion-warning] ${this.formatError(error)}`);
      return fsPath;
    }
  }

  shellQuote(value) {
    return `'${String(value).replace(/'/g, `'\"'\"'`)}'`;
  }

  terminalQuote(value) {
    return `"${String(value).replace(/"/g, '\\"')}"`;
  }

  powerShellQuote(value) {
    return `'${String(value).replace(/'/g, "''")}'`;
  }

  cmdQuote(value) {
    return `"${String(value).replace(/"/g, '""')}"`;
  }

  recordActionResult(label, success, output) {
    this.state.lastAction = {
      label,
      success,
      output: (output || "").trim() || "(no output)",
      time: new Date().toLocaleTimeString()
    };
    this.postState();
  }

  combineOutput(stdout, stderr) {
    return [stdout, stderr].filter(Boolean).join("\n").trim();
  }

  firstUsefulLine(text) {
    return text
      .split(/\r?\n/u)
      .map(line => line.trim())
      .filter(line => line.length > 0 && !line.startsWith("==="))[0] || "";
  }

  formatError(error) {
    if (!error) {
      return "Unknown error";
    }

    if (typeof error === "string") {
      return error;
    }

    return error.message || String(error);
  }

  renderHtml(webview) {
    const nonce = crypto.randomBytes(16).toString("base64");
    const iconUri = webview.asWebviewUri(vscode.Uri.file(path.join(__dirname, "media", "icon.svg")));

    return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src ${webview.cspSource} https: data:; style-src 'nonce-${nonce}'; script-src 'nonce-${nonce}';" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>WAR Ops: Dashboard</title>
  <style nonce="${nonce}">
    :root {
      color-scheme: dark;
      --bg: var(--vscode-sideBar-background);
      --panel: var(--vscode-editorWidget-background);
      --panel-alt: color-mix(in srgb, var(--vscode-editorWidget-background) 80%, transparent);
      --border: var(--vscode-widget-border, var(--vscode-panel-border));
      --text: var(--vscode-foreground);
      --muted: var(--vscode-descriptionForeground);
      --accent: var(--vscode-textLink-foreground);
      --ok: #4bc17e;
      --warn: #d0a94c;
      --err: #de6b6b;
      --shadow: 0 10px 30px rgba(0, 0, 0, 0.18);
    }

    * {
      box-sizing: border-box;
    }

    body {
      margin: 0;
      font: 11px/1.45 var(--vscode-font-family);
      color: var(--text);
      background: linear-gradient(180deg, color-mix(in srgb, var(--bg) 88%, black), var(--bg));
    }

    .shell {
      display: flex;
      flex-direction: column;
      gap: 10px;
      padding: 10px;
    }

    .top-strip {
      position: sticky;
      top: 0;
      z-index: 20;
      display: flex;
      flex-direction: column;
      gap: 8px;
      padding-bottom: 6px;
      background: linear-gradient(180deg, var(--bg) 0%, color-mix(in srgb, var(--bg) 92%, transparent) 100%);
      backdrop-filter: blur(12px);
    }

    .hero {
      display: grid;
      grid-template-columns: auto 1fr;
      gap: 10px;
      align-items: center;
      padding: 10px 12px;
      border: 1px solid var(--border);
      border-radius: 12px;
      background: linear-gradient(135deg, color-mix(in srgb, var(--panel) 92%, black), color-mix(in srgb, var(--panel) 74%, transparent));
      box-shadow: var(--shadow);
    }

    .hero img {
      width: 28px;
      height: 28px;
    }

    .hero h1 {
      margin: 0;
      font-size: 13px;
      letter-spacing: 0.04em;
    }

    .hero p {
      margin: 2px 0 0;
      font-size: 10px;
      color: var(--muted);
    }

    .action-card {
      font: inherit;
      color: inherit;
      background: none;
    }

    .action-card:hover {
      border-color: color-mix(in srgb, var(--accent) 55%, var(--border));
    }

    .action-card:disabled {
      opacity: 0.6;
      cursor: default;
    }

    .busy-banner {
      display: none;
      padding: 7px 10px;
      border: 1px solid color-mix(in srgb, var(--accent) 30%, var(--border));
      border-radius: 10px;
      background: color-mix(in srgb, var(--accent) 10%, var(--panel));
      color: var(--text);
    }

    .busy-banner.visible {
      display: block;
    }

    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(145px, 1fr));
      gap: 8px;
    }

    .status-card {
      display: grid;
      grid-template-columns: auto 1fr;
      gap: 8px;
      align-items: center;
      padding: 8px 10px;
      border: 1px solid var(--border);
      border-radius: 12px;
      background: color-mix(in srgb, var(--panel) 86%, transparent);
      min-height: 64px;
    }

    .dial {
      width: 38px;
      height: 38px;
      border-radius: 999px;
      display: grid;
      place-items: center;
      font-size: 8px;
      font-weight: 700;
      letter-spacing: 0.08em;
      border: 2px solid var(--muted);
      color: var(--text);
      background: radial-gradient(circle at 30% 30%, rgba(255,255,255,0.08), rgba(0,0,0,0.18));
    }

    .status-card[data-tone="healthy"] .dial { border-color: var(--ok); }
    .status-card[data-tone="warning"] .dial { border-color: var(--warn); }
    .status-card[data-tone="error"] .dial { border-color: var(--err); }

    .status-card h2 {
      margin: 0;
      font-size: 11px;
    }

    .status-card p {
      margin: 2px 0 0;
      font-size: 10px;
      color: var(--muted);
    }

    .meta-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 8px;
    }

    details.meta-section {
      border: 1px solid var(--border);
      border-radius: 12px;
      background: color-mix(in srgb, var(--panel) 78%, transparent);
      overflow: hidden;
    }

    details.meta-section > summary {
      list-style: none;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      padding: 9px 10px;
      background: color-mix(in srgb, var(--panel) 88%, black);
    }

    details.meta-section > summary::-webkit-details-marker {
      display: none;
    }

    .meta-summary {
      display: flex;
      flex-direction: column;
      gap: 2px;
    }

    .meta-summary h2 {
      margin: 0;
      font-size: 11px;
    }

    .meta-summary span {
      font-size: 10px;
      color: var(--muted);
    }

    .meta-body {
      padding: 10px;
      border-top: 1px solid color-mix(in srgb, var(--border) 70%, transparent);
    }

    .path-list,
    .last-action {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .last-action {
      min-width: 0;
    }

    .last-action-summary {
      display: flex;
      flex-wrap: wrap;
      gap: 6px;
      align-items: center;
    }

    .last-action-output {
      margin: 0;
      min-width: 0;
      max-height: 168px;
      padding: 8px 9px;
      border: 1px solid color-mix(in srgb, var(--border) 82%, transparent);
      border-radius: 10px;
      background: color-mix(in srgb, var(--panel) 70%, black);
      color: var(--text);
      font: 9px/1.45 var(--vscode-editor-font-family, var(--vscode-font-family));
      white-space: pre;
      overflow-x: auto;
      overflow-y: auto;
      tab-size: 2;
    }

    .pill {
      display: inline-flex;
      width: fit-content;
      max-width: 100%;
      align-items: center;
      gap: 4px;
      padding: 3px 7px;
      font-size: 10px;
      border-radius: 999px;
      background: color-mix(in srgb, var(--panel) 72%, black);
      color: var(--muted);
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }

    details.section {
      border: 1px solid var(--border);
      border-radius: 12px;
      background: color-mix(in srgb, var(--panel) 80%, transparent);
      overflow: hidden;
    }

    details.section > summary {
      list-style: none;
      cursor: pointer;
      padding: 10px 12px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      background: color-mix(in srgb, var(--panel) 90%, black);
    }

    details.section > summary::-webkit-details-marker {
      display: none;
    }

    .summary-copy {
      display: flex;
      flex-direction: column;
      gap: 3px;
    }

    .summary-copy strong {
      font-size: 12px;
    }

    .summary-copy span {
      font-size: 10px;
      color: var(--muted);
    }

    .section-body {
      padding: 10px 12px 12px;
      display: flex;
      flex-direction: column;
      gap: 10px;
    }

    .group {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .group-title {
      font-size: 10px;
      font-weight: 700;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      color: var(--muted);
    }

    .action-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(190px, 1fr));
      gap: 8px;
    }

    .action-card {
      display: flex;
      flex-direction: column;
      gap: 4px;
      align-items: flex-start;
      text-align: left;
      width: 100%;
      padding: 9px 10px;
      border-radius: 12px;
      border: 1px solid var(--border);
      background: linear-gradient(180deg, color-mix(in srgb, var(--panel) 92%, transparent), color-mix(in srgb, var(--panel) 72%, black));
      cursor: pointer;
    }

    .action-card[data-tone="danger"] {
      border-color: color-mix(in srgb, var(--err) 45%, var(--border));
    }

    .action-card strong {
      font-size: 11px;
    }

    .action-card span {
      font-size: 10px;
      color: var(--muted);
    }

    .section-meta {
      font-size: 10px;
      color: var(--muted);
      white-space: nowrap;
    }
  </style>
</head>
<body>
  <div class="shell">
    <div class="top-strip">
      <div class="hero">
        <img src="${iconUri}" alt="" />
        <div>
          <h1>WAR Ops: Dashboard</h1>
          <p>Compact control surface for build, package, host, client, and milestone validation workflows.</p>
        </div>
      </div>

      <div class="busy-banner" id="busyBanner"></div>
      <div class="status-grid" id="statusGrid"></div>
      <div class="meta-grid">
        <details class="meta-section">
          <summary>
            <div class="meta-summary">
              <h2>Resolved Paths</h2>
              <span>Open when you need to sanity-check workspace and package routing.</span>
            </div>
            <span class="section-meta">expand</span>
          </summary>
          <div class="meta-body">
            <div class="path-list" id="pathList"></div>
          </div>
        </details>
        <details class="meta-section">
          <summary>
            <div class="meta-summary">
              <h2>Last Action</h2>
              <span>Recent command result and timestamp.</span>
            </div>
            <span class="section-meta">expand</span>
          </summary>
          <div class="meta-body">
            <div class="last-action" id="lastAction"></div>
          </div>
        </details>
      </div>
    </div>

    <div id="sections"></div>
  </div>

  <script nonce="${nonce}">
    const vscode = acquireVsCodeApi();
    const initialState = ${JSON.stringify(this.buildViewState())};
    const persistedViewState = vscode.getState() || {};

    let currentState = initialState;
    let uiState = {
      openSections: Array.isArray(persistedViewState.openSections) ? persistedViewState.openSections : null
    };

    function escapeHtml(value) {
      return String(value ?? "")
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#39;");
    }

    function toneLabel(tone) {
      switch (tone) {
        case "healthy": return "GOOD";
        case "warning": return "WARN";
        case "error": return "BAD";
        default: return "WAIT";
      }
    }

    function renderBusy(state) {
      const banner = document.getElementById("busyBanner");
      if (state.busy) {
        banner.classList.add("visible");
        banner.textContent = "In progress: " + state.busy;
      } else {
        banner.classList.remove("visible");
        banner.textContent = "";
      }
    }

    function renderStatuses(state) {
      const statusGrid = document.getElementById("statusGrid");
      statusGrid.innerHTML = state.statuses.map(status => \`
        <article class="status-card" data-tone="\${escapeHtml(status.tone)}">
          <div class="dial">\${toneLabel(status.tone)}</div>
          <div>
            <h2>\${escapeHtml(status.label)}</h2>
            <p>\${escapeHtml(status.detail)}</p>
          </div>
        </article>
      \`).join("");
    }

    function renderPaths(state) {
      const pathList = document.getElementById("pathList");
      if (!state.paths) {
        pathList.innerHTML = '<span class="pill">Waiting for first refresh</span>';
        return;
      }

      pathList.innerHTML = [
        ["Workspace", state.paths.workspaceRoot],
        ["Repo", state.paths.repoRoot],
        ["Stage", state.paths.stageRoot],
        ["Runtime", state.paths.runtimeRoot],
        ["Last refresh", state.lastRefresh]
      ].map(([label, value]) => \`<span class="pill"><strong>\${escapeHtml(label)}:</strong>&nbsp;\${escapeHtml(value)}</span>\`).join("");
    }

    function renderLastAction(state) {
      const container = document.getElementById("lastAction");
      if (!state.lastAction) {
        container.innerHTML = '<span class="pill">No action has been run yet.</span>';
        return;
      }

      const marker = state.lastAction.success ? "OK" : "FAILED";
      container.innerHTML = \`
        <div class="last-action-summary">
          <span class="pill"><strong>\${escapeHtml(state.lastAction.label)}</strong>&nbsp;\${escapeHtml(marker)} at \${escapeHtml(state.lastAction.time)}</span>
        </div>
        <pre class="last-action-output">\${escapeHtml(state.lastAction.output)}</pre>
      \`;
    }

    function persistUiState() {
      vscode.setState(uiState);
    }

    function snapshotOpenSections() {
      return Array.from(document.querySelectorAll("#sections details.section[data-section-id]"))
        .filter(section => section.open)
        .map(section => section.dataset.sectionId);
    }

    function resolveOpenSections(state) {
      const validIds = new Set(state.sections.map(section => section.id));
      const savedIds = Array.isArray(uiState.openSections)
        ? uiState.openSections.filter(sectionId => validIds.has(sectionId))
        : null;

      if (savedIds !== null) {
        return savedIds;
      }

      return state.sections[0] ? [state.sections[0].id] : [];
    }

    function attachSectionListeners() {
      for (const section of document.querySelectorAll("#sections details.section[data-section-id]")) {
        section.addEventListener("toggle", () => {
          uiState.openSections = snapshotOpenSections();
          persistUiState();
        });
      }
    }

    function renderSections(state) {
      const sections = document.getElementById("sections");
      const existingOpenSections = snapshotOpenSections();
      if (existingOpenSections.length > 0 || document.querySelector("#sections details.section")) {
        uiState.openSections = existingOpenSections;
      }

      const openSections = new Set(resolveOpenSections(state));

      sections.innerHTML = state.sections.map(section => \`
        <details class="section" data-section-id="\${escapeHtml(section.id)}" \${openSections.has(section.id) ? "open" : ""}>
          <summary>
            <div class="summary-copy">
              <strong>\${escapeHtml(section.title)}</strong>
              <span>\${escapeHtml(section.description)}</span>
            </div>
            <span class="section-meta">\${section.groups.reduce((count, group) => count + group.actions.length, 0)} actions</span>
          </summary>
          <div class="section-body">
            \${section.groups.map(group => \`
              <div class="group">
                <div class="group-title">\${escapeHtml(group.title)}</div>
                <div class="action-grid">
                  \${group.actions.map(action => \`
                    <button class="action-card" data-action-id="\${escapeHtml(action.id)}" data-tone="\${escapeHtml(action.tone)}" \${state.busy ? "disabled" : ""}>
                      <strong>\${escapeHtml(action.label)}</strong>
                      <span>\${escapeHtml(action.description)}</span>
                    </button>
                  \`).join("")}
                </div>
              </div>
            \`).join("")}
          </div>
        </details>
      \`).join("");

      attachSectionListeners();
    }

    function render(state) {
      currentState = state;
      renderBusy(state);
      renderStatuses(state);
      renderPaths(state);
      renderLastAction(state);
      renderSections(state);
    }

    document.addEventListener("click", event => {
      const target = event.target.closest("[data-action-id]");
      if (!target || target.disabled) {
        return;
      }

      vscode.postMessage({ type: "action", actionId: target.dataset.actionId });
    });

    window.addEventListener("message", event => {
      if (event.data?.type === "state") {
        render(event.data.state);
      }
    });

    render(initialState);
    vscode.postMessage({ type: "ready" });
  </script>
</body>
</html>`;
  }
}

function activate(context) {
  const provider = new WarOperationsDashboardProvider();
  provider.initialize();
  const actionCommandRegistrations = Object.keys(ACTIONS)
    .filter(actionId => !ACTION_COMMAND_EXCLUSIONS.has(actionId))
    .map(actionId => {
      return vscode.commands.registerCommand(toActionCommandId(actionId), async () => {
        await provider.handleAction(actionId);
      });
    });

  context.subscriptions.push(
    provider,
    vscode.window.registerWebviewViewProvider(VIEW_TYPE, provider, {
      webviewOptions: {
        retainContextWhenHidden: true
      }
    }),
    vscode.commands.registerCommand("warOps.openDashboard", async () => provider.focusSidebar()),
    vscode.commands.registerCommand("warOps.focusPanel", async () => provider.focusSidebar()),
    vscode.commands.registerCommand("warOps.refresh", async () => provider.refreshStatuses(true)),
    vscode.commands.registerCommand("warOps.openOutput", async () => provider.output.show(true)),
    ...actionCommandRegistrations
  );
}

function deactivate() {
  return undefined;
}

module.exports = {
  activate,
  deactivate
};
