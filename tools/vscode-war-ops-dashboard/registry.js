function scriptAction(id, label, description, options = {}) {
  return {
    id,
    label,
    description,
    kind: "script",
    tone: "normal",
    root: "repo",
    relativePath: "",
    args: [],
    terminal: false,
    background: false,
    ...options
  };
}

function openFileAction(id, label, description, options = {}) {
  return {
    id,
    label,
    description,
    kind: "openFile",
    relativePath: "",
    ...options
  };
}

function utilityAction(id, label, description, options = {}) {
  return {
    id,
    label,
    description,
    kind: "utility",
    tone: "normal",
    ...options
  };
}

const STATUS_DEFINITIONS = [
  {
    id: "stagePackage",
    label: "Internal Alpha",
    kind: "stagePackage"
  },
  {
    id: "hostSmoke",
    label: "Host Smoke",
    kind: "report",
    relativePath: "Logs/m45_headless_host_smoke_test.txt"
  },
  {
    id: "clientSmoke",
    label: "Client Smoke",
    kind: "report",
    relativePath: "Logs/m45_local_demo_smoke_test.txt"
  },
  {
    id: "runtimeSurface",
    label: "Runtime",
    kind: "runtimeFiles",
    relativePaths: [
      "Host/headless_host_status.txt",
      "Host/authoritative_snapshot.txt",
      "Logs/client_replication_status.txt",
      "Saves/authoritative_world_primary.txt"
    ]
  }
];

const ACTIONS = {
  refreshDashboard: utilityAction(
    "refreshDashboard",
    "Refresh",
    "Refresh the dashboard status cards now.",
    { utility: "refresh" }
  ),
  openOutput: utilityAction(
    "openOutput",
    "Open Output",
    "Show the WAR Ops output channel.",
    { utility: "output" }
  ),

  packageInternalAlphaRelease: scriptAction(
    "packageInternalAlphaRelease",
    "Package Release",
    "Build and stage the internal-alpha release package.",
    {
      relativePath: "Project/scripts/build_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),
  validateInternalAlphaRelease: scriptAction(
    "validateInternalAlphaRelease",
    "Validate Package",
    "Run the aggregate internal-alpha package validation lane.",
    {
      relativePath: "Project/scripts/validate_m48_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),

  launchHeadlessHost: scriptAction(
    "launchHeadlessHost",
    "Launch Host",
    "Start the staged headless host.",
    {
      root: "stage",
      terminal: true,
      relativePath: "launch_headless_host_win64.bat"
    }
  ),
  launchLocalDemo: scriptAction(
    "launchLocalDemo",
    "Launch Client",
    "Start the staged WAR desktop client.",
    {
      root: "stage",
      terminal: true,
      relativePath: "launch_local_demo_win64.bat"
    }
  ),
  smokeHost: scriptAction(
    "smokeHost",
    "Smoke Host",
    "Run the staged headless-host smoke test.",
    {
      root: "stage",
      relativePath: "smoke_test_headless_host_win64.bat"
    }
  ),
  smokeLocalDemo: scriptAction(
    "smokeLocalDemo",
    "Smoke Client",
    "Run the staged client smoke test.",
    {
      root: "stage",
      relativePath: "smoke_test_local_demo_win64.bat"
    }
  ),

  openAlphaHandoff: openFileAction(
    "openAlphaHandoff",
    "Alpha Handoff",
    "Open the single alpha handoff document.",
    {
      relativePath: "Docs/Planning/WAR Alpha Handoff.md"
    }
  ),
  openTechnicalArchitecture: openFileAction(
    "openTechnicalArchitecture",
    "Technical Architecture",
    "Open the canonical technical architecture document.",
    {
      relativePath: "Docs/Technical/WAR Technical Architecture.md"
    }
  )
};

const SECTIONS = [
  {
    id: "build",
    title: "Build",
    description: "Package and validate the canonical internal-alpha lane.",
    groups: [
      {
        title: "Package",
        actionIds: [
          "packageInternalAlphaRelease",
          "validateInternalAlphaRelease"
        ]
      }
    ]
  },
  {
    id: "run",
    title: "Run",
    description: "Launch the staged host and client without leaving VS Code.",
    groups: [
      {
        title: "Runtime",
        actionIds: [
          "launchHeadlessHost",
          "launchLocalDemo"
        ]
      }
    ]
  },
  {
    id: "check",
    title: "Check",
    description: "Refresh status, review output, and run the two smoke checks.",
    groups: [
      {
        title: "Verification",
        actionIds: [
          "refreshDashboard",
          "openOutput",
          "smokeHost",
          "smokeLocalDemo"
        ]
      }
    ]
  },
  {
    id: "docs",
    title: "Docs",
    description: "Open the small canonical documentation set.",
    groups: [
      {
        title: "Core Docs",
        actionIds: [
          "openAlphaHandoff",
          "openTechnicalArchitecture"
        ]
      }
    ]
  }
];

module.exports = {
  ACTIONS,
  SECTIONS,
  STATUS_DEFINITIONS
};
