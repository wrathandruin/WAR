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
    label: "Staged Package",
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
    id: "hostedBootstrap",
    label: "M45 Bootstrap",
    kind: "report",
    root: "stage",
    relativePath: "HostedBootstrapRuntime/Logs/m45_hosted_bootstrap_validation.txt"
  },
  {
    id: "m46Validation",
    label: "M46 Baseline",
    kind: "bundleReport",
    root: "stage",
    relativePaths: [
      "M46HostedEnvironmentRuntime/Logs/m46_environment_identity_validation.txt",
      "M46MissingSecretsRuntime/Logs/m46_missing_required_secrets_validation.txt",
      "M46RuntimeOwnershipRuntime/Logs/m46_runtime_save_hygiene_validation.txt"
    ]
  },
  {
    id: "m47Validation",
    label: "M47 Session",
    kind: "bundleReport",
    root: "stage",
    relativePaths: [
      "M47TicketIssueRuntime/Logs/m47_ticket_issue_and_client_entry_validation.txt",
      "M47TicketDenialRuntime/Logs/m47_ticket_denial_and_fail_states_validation.txt",
      "M47ReconnectRuntime/Logs/m47_reconnect_identity_validation.txt"
    ]
  },
  {
    id: "m48Validation",
    label: "M48 Triage",
    kind: "bundleReport",
    root: "stage",
    relativePaths: [
      "M48FailureBundleRuntime/Logs/m48_failure_bundle_capture_validation.txt",
      "M48OperatorTriageRuntime/Logs/m48_operator_triage_validation.txt"
    ]
  },
  {
    id: "m49Validation",
    label: "M49 Beta",
    kind: "bundleReport",
    root: "stage",
    relativePaths: [
      "runtime/Logs/m49_beta_content_scale_validation.txt",
      "M49RetentionRuntime/Logs/m49_retention_expansion_validation.txt",
      "M49ReleaseCandidateRuntime/Logs/m49_release_candidate_discipline_validation.txt"
    ]
  },
  {
    id: "m50Validation",
    label: "M50 Market",
    kind: "bundleReport",
    root: "stage",
    relativePaths: [
      "M50LauncherRuntime/Logs/m50_launcher_validation.txt",
      "M50InstallerRuntime/Logs/m50_installer_validation.txt",
      "M50UpdateRuntime/Logs/m50_update_validation.txt"
    ]
  },
  {
    id: "runtimeSurface",
    label: "Runtime Surface",
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
    "Refresh Status",
    "Refresh the dashboard status cards now.",
    { utility: "refresh" }
  ),
  openOutput: utilityAction(
    "openOutput",
    "Open Output",
    "Show the WAR Ops output channel.",
    { utility: "output" }
  ),

  buildPackageRelease: scriptAction(
    "buildPackageRelease",
    "Package Internal Alpha",
    "Build and stage the internal-alpha release package.",
    {
      relativePath: "scripts/build_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),
  buildPackageDebug: scriptAction(
    "buildPackageDebug",
    "Package IA Debug",
    "Build and stage the internal-alpha debug package.",
    {
      relativePath: "scripts/build_internal_alpha_package_win64.bat",
      args: ["Debug"]
    }
  ),
  buildBetaCandidateRelease: scriptAction(
    "buildBetaCandidateRelease",
    "Package Beta",
    "Build and stage the beta-candidate release package.",
    {
      relativePath: "scripts/build_beta_release_candidate_package_win64.bat",
      args: ["Release"]
    }
  ),
  buildMarketCandidateRelease: scriptAction(
    "buildMarketCandidateRelease",
    "Package Market",
    "Build and stage the market-candidate release package.",
    {
      relativePath: "scripts/build_market_candidate_package_win64.bat",
      args: ["Release"]
    }
  ),
  buildShadersDebug: scriptAction(
    "buildShadersDebug",
    "Build Shaders Debug",
    "Regenerate bgfx shader binaries for the debug lane.",
    {
      relativePath: "scripts/build_shaders_win64.bat",
      args: ["{repoRoot}", "Debug"]
    }
  ),
  buildShadersRelease: scriptAction(
    "buildShadersRelease",
    "Build Shaders Release",
    "Regenerate bgfx shader binaries for the release lane.",
    {
      relativePath: "scripts/build_shaders_win64.bat",
      args: ["{repoRoot}", "Release"]
    }
  ),

  launchHeadlessHost: scriptAction(
    "launchHeadlessHost",
    "Launch Headless Host",
    "Start the staged host executable with the packaged runtime layout.",
    {
      root: "stage",
      terminal: true,
      relativePath: "launch_headless_host_win64.bat"
    }
  ),
  launchLocalDemo: scriptAction(
    "launchLocalDemo",
    "Launch Local Demo",
    "Start the staged WAR desktop client.",
    {
      root: "stage",
      terminal: true,
      relativePath: "launch_local_demo_win64.bat"
    }
  ),
  launchClientAgainstHost: scriptAction(
    "launchClientAgainstHost",
    "Launch Client + Host",
    "Start the staged host and then the client together.",
    {
      root: "stage",
      terminal: true,
      relativePath: "launch_local_client_against_host_win64.bat"
    }
  ),

  smokeHost: scriptAction(
    "smokeHost",
    "Smoke Host",
    "Run the packaged headless-host smoke test.",
    {
      root: "stage",
      relativePath: "smoke_test_headless_host_win64.bat"
    }
  ),
  smokeLocalDemo: scriptAction(
    "smokeLocalDemo",
    "Smoke Client",
    "Run the packaged client smoke test.",
    {
      root: "stage",
      relativePath: "smoke_test_local_demo_win64.bat"
    }
  ),
  validateM45InternalAlpha: scriptAction(
    "validateM45InternalAlpha",
    "Validate M45",
    "Run the repo-owned M45 aggregate validation lane.",
    {
      relativePath: "scripts/validate_m45_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),
  validateM46InternalAlpha: scriptAction(
    "validateM46InternalAlpha",
    "Validate M46",
    "Run the repo-owned M46 aggregate validation lane.",
    {
      relativePath: "scripts/validate_m46_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),
  validateM47InternalAlpha: scriptAction(
    "validateM47InternalAlpha",
    "Validate M47",
    "Run the repo-owned M47 aggregate validation lane.",
    {
      relativePath: "scripts/validate_m47_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),
  validateM48InternalAlpha: scriptAction(
    "validateM48InternalAlpha",
    "Validate M48",
    "Run the repo-owned M48 aggregate validation lane.",
    {
      relativePath: "scripts/validate_m48_internal_alpha_package_win64.bat",
      args: ["Release"]
    }
  ),
  validateM49BetaCandidate: scriptAction(
    "validateM49BetaCandidate",
    "Validate M49",
    "Run the repo-owned M49 beta-candidate validation lane.",
    {
      relativePath: "scripts/validate_m49_beta_candidate_package_win64.bat",
      args: ["Release"]
    }
  ),
  validateM50MarketCandidate: scriptAction(
    "validateM50MarketCandidate",
    "Validate M50",
    "Run the repo-owned M50 market-candidate validation lane.",
    {
      relativePath: "scripts/validate_m50_market_candidate_package_win64.bat",
      args: ["Release"]
    }
  ),

  openReadme: openFileAction(
    "openReadme",
    "README",
    "Open the current repo milestone summary and manual validation notes.",
    {
      relativePath: "README.md"
    }
  ),
  openRuntimeBoundary: openFileAction(
    "openRuntimeBoundary",
    "Runtime Boundary",
    "Open the runtime boundary contract.",
    {
      relativePath: "Docs/Technical/Wrath and Ruin - Runtime Boundary Contract.md"
    }
  ),
  openRunway: openFileAction(
    "openRunway",
    "M45-M70 Runway",
    "Open the current alpha playtest runway.",
    {
      relativePath: "Docs/Planning/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md"
    }
  ),
  openM48Milestone: openFileAction(
    "openM48Milestone",
    "M48 Milestone",
    "Open the M48 milestone document.",
    {
      relativePath: "Milestones/M48_Crash_Capture_Failure_Bundles_Operator_Triage_Baseline.md"
    }
  ),
  openM49Milestone: openFileAction(
    "openM49Milestone",
    "M49 Milestone",
    "Open the M49 milestone document.",
    {
      relativePath: "Milestones/M49_Beta_Content_Scale_Retention_Expansion_Release_Candidate_Discipline.md"
    }
  ),
  openM50Milestone: openFileAction(
    "openM50Milestone",
    "M50 Milestone",
    "Open the M50 milestone document.",
    {
      relativePath: "Milestones/M50_Launcher_Installer_Update_Productionization.md"
    }
  ),
  openM48Evidence: openFileAction(
    "openM48Evidence",
    "M48 Evidence",
    "Open the M48 validation evidence file.",
    {
      relativePath: "Docs/Validation/Docs/Validation/VALIDATION_EVIDENCE_M48.txt"
    }
  ),
  openM49Evidence: openFileAction(
    "openM49Evidence",
    "M49 Evidence",
    "Open the M49 validation evidence file.",
    {
      relativePath: "Docs/Validation/Docs/Validation/VALIDATION_EVIDENCE_M49.txt"
    }
  ),
  openM50Evidence: openFileAction(
    "openM50Evidence",
    "M50 Evidence",
    "Open the M50 validation evidence file.",
    {
      relativePath: "Docs/Validation/Docs/Validation/VALIDATION_EVIDENCE_M50.txt"
    }
  ),
  openRoadmap: openFileAction(
    "openRoadmap",
    "Roadmap",
    "Open the strategic milestone roadmap.",
    {
      relativePath: "Milestones/WAR_Strategic_Roadmap.md"
    }
  )
};

const TOP_ACTION_IDS = [
  "refreshDashboard",
  "openOutput",
  "buildMarketCandidateRelease",
  "openM50Milestone"
];

const SECTIONS = [
  {
    id: "build",
    title: "Build + Package",
    description: "Compile and stage the internal-alpha, beta-candidate, and market-candidate lanes from one place.",
    groups: [
      {
        title: "Packages",
        actionIds: [
          "buildPackageRelease",
          "buildPackageDebug",
          "buildBetaCandidateRelease",
          "buildMarketCandidateRelease"
        ]
      },
      {
        title: "Shaders",
        actionIds: ["buildShadersDebug", "buildShadersRelease"]
      }
    ]
  },
  {
    id: "launch",
    title: "Launch",
    description: "Start the latest staged host and client lanes from the current repo state.",
    groups: [
      {
        title: "Runtime",
        actionIds: ["launchHeadlessHost", "launchLocalDemo", "launchClientAgainstHost"]
      }
    ]
  },
  {
    id: "validation",
    title: "Validation",
    description: "Run the smoke and milestone validation gates that matter right now.",
    groups: [
      {
        title: "Smoke",
        actionIds: ["smokeHost", "smokeLocalDemo"]
      },
      {
        title: "Milestone Gates",
        actionIds: [
          "validateM45InternalAlpha",
          "validateM46InternalAlpha",
          "validateM47InternalAlpha",
          "validateM48InternalAlpha",
          "validateM49BetaCandidate",
          "validateM50MarketCandidate"
        ]
      }
    ]
  },
  {
    id: "docs",
    title: "Docs",
    description: "Open the production docs, milestone handoffs, and validation evidence without leaving the dashboard.",
    groups: [
      {
        title: "Core Guides",
        actionIds: [
          "openReadme",
          "openRuntimeBoundary",
          "openRunway",
          "openRoadmap"
        ]
      },
      {
        title: "Later Milestones",
        actionIds: [
          "openM48Milestone",
          "openM49Milestone",
          "openM50Milestone",
          "openM48Evidence",
          "openM49Evidence",
          "openM50Evidence"
        ]
      }
    ]
  }
];

module.exports = {
  ACTIONS,
  SECTIONS,
  STATUS_DEFINITIONS,
  TOP_ACTION_IDS
};
