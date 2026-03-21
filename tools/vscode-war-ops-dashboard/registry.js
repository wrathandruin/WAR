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
    kind: "stagePackage",
    requiredPaths: ["WAR.exe", "WARServer.exe", "demo_manifest.txt"]
  },
  {
    id: "hostSmoke",
    label: "Host Smoke",
    kind: "report",
    relativePath: "Logs/headless_host_smoke_test.txt"
  },
  {
    id: "clientSmoke",
    label: "Client Smoke",
    kind: "report",
    relativePath: "Logs/local_demo_smoke_test.txt"
  },
  {
    id: "acceptanceM40",
    label: "M40 Acceptance",
    kind: "report",
    relativePath: "Logs/m40_combat_acceptance_report.txt"
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
    "Package Release",
    "Build and stage the local demo release package.",
    {
      relativePath: "scripts/build_local_demo_package_win64.bat",
      args: ["Release"]
    }
  ),
  buildPackageDebug: scriptAction(
    "buildPackageDebug",
    "Package Debug",
    "Build and stage the local demo debug package.",
    {
      relativePath: "scripts/build_local_demo_package_win64.bat",
      args: ["Debug"]
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
    "Run the packaged local-demo smoke test.",
    {
      root: "stage",
      relativePath: "smoke_test_local_demo_win64.bat"
    }
  ),
  acceptanceM36: scriptAction(
    "acceptanceM36",
    "Acceptance M36",
    "Run the localhost authority acceptance lane.",
    {
      root: "stage",
      relativePath: "acceptance_m36_localhost_authority_win64.bat"
    }
  ),
  acceptanceM37: scriptAction(
    "acceptanceM37",
    "Acceptance M37",
    "Run the persistence schema and save-load acceptance lane.",
    {
      root: "stage",
      relativePath: "acceptance_m37_persistence_win64.bat"
    }
  ),
  acceptanceM38: scriptAction(
    "acceptanceM38",
    "Acceptance M38",
    "Run the persistence plus inventory acceptance lane.",
    {
      root: "stage",
      relativePath: "acceptance_m38_persistence_inventory_win64.bat"
    }
  ),
  acceptanceM39: scriptAction(
    "acceptanceM39",
    "Acceptance M39",
    "Run the survival hazards acceptance lane.",
    {
      root: "stage",
      relativePath: "acceptance_m39_survival_hazards_win64.bat"
    }
  ),
  acceptanceM40: scriptAction(
    "acceptanceM40",
    "Acceptance M40",
    "Run the six-second combat acceptance lane.",
    {
      root: "stage",
      relativePath: "acceptance_m40_six_second_combat_win64.bat"
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
      relativePath: "Docs/Wrath and Ruin - Runtime Boundary Contract.md"
    }
  ),
  openLocalDemoGuide: openFileAction(
    "openLocalDemoGuide",
    "Local Demo Guide",
    "Open the packaging and local demo lane baseline guide.",
    {
      relativePath: "Docs/Wrath and Ruin - Local Demo Lane and Packaging Baseline.md"
    }
  ),
  openM37Readiness: openFileAction(
    "openM37Readiness",
    "M37 Readiness",
    "Open the M37 readiness and handoff report.",
    {
      relativePath: "Docs/Wrath and Ruin - M37 Readiness Report.md"
    }
  ),
  openM40Brief: openFileAction(
    "openM40Brief",
    "M40 Brief",
    "Open the M40 combat runtime and encounter brief.",
    {
      relativePath: "Docs/Wrath and Ruin - M40 Combat Runtime and Encounter Resolution Brief.md"
    }
  ),
  openM40Checklist: openFileAction(
    "openM40Checklist",
    "M40 Checklist",
    "Open the M40 validation and acceptance checklist.",
    {
      relativePath: "Docs/Wrath and Ruin - M40 Validation and Acceptance Checklist.md"
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
  "openReadme",
  "openM40Checklist"
];

const SECTIONS = [
  {
    id: "build",
    title: "Build + Package",
    description: "Compile the staged demo and regenerate the shader lane without hunting through scripts.",
    groups: [
      {
        title: "Package",
        actionIds: ["buildPackageRelease", "buildPackageDebug"]
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
    description: "Start the packaged host and client lanes from the current repo state.",
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
    description: "Run the smoke and milestone acceptance gates that matter right now.",
    groups: [
      {
        title: "Smoke",
        actionIds: ["smokeHost", "smokeLocalDemo"]
      },
      {
        title: "Acceptance",
        actionIds: [
          "acceptanceM36",
          "acceptanceM37",
          "acceptanceM38",
          "acceptanceM39",
          "acceptanceM40"
        ]
      }
    ]
  },
  {
    id: "docs",
    title: "Docs",
    description: "Open the production docs and milestone guidance without leaving the dashboard.",
    groups: [
      {
        title: "Core Guides",
        actionIds: [
          "openReadme",
          "openRuntimeBoundary",
          "openLocalDemoGuide",
          "openM37Readiness",
          "openRoadmap"
        ]
      },
      {
        title: "Current Milestone",
        actionIds: ["openM40Brief", "openM40Checklist"]
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
