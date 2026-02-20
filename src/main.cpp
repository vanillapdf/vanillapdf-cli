#include <CLI/CLI.hpp>
#include <vanillapdf/c_vanillapdf_api.h>

#include <iostream>
#include <map>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

static const std::map<std::string, LoggingSeverity> log_levels = {
    {"off",      LoggingSeverity_Off},
    {"critical", LoggingSeverity_Critical},
    {"error",    LoggingSeverity_Error},
    {"warning",  LoggingSeverity_Warning},
    {"info",     LoggingSeverity_Info},
    {"debug",    LoggingSeverity_Debug},
    {"trace",    LoggingSeverity_Trace},
};

static constexpr const char* copyright_notice =
    "Copyright 2026 Vanilla.PDF Labs\n"
    "Licensed under the Apache License, Version 2.0\n"
    "https://www.apache.org/licenses/LICENSE-2.0\n";

int main(int argc, char** argv) {
    Logging_SetSeverity(LoggingSeverity_Off);

    CLI::App app{"vanillapdf-cli - PDF manipulation tool"};
    app.set_version_flag("--version", VANILLAPDF_CLI_VERSION);
    app.add_flag_callback("--copyright", []() {
        std::cout << copyright_notice;
        std::exit(0);
    }, "Show copyright and license information");
    app.require_subcommand(1);
    app.fallthrough();

    LoggingSeverity log_severity = LoggingSeverity_Off;
    app.add_option("--log-level", log_severity, "Set library log level (off, critical, error, warning, info, debug, trace)")
        ->transform(CLI::CheckedTransformer(log_levels));
    app.add_flag("-v,--verbose", vpdf::g_verbose, "Show detailed error information");

    app.parse_complete_callback([&log_severity]() {
        Logging_SetSeverity(log_severity);
    });

    vpdf::commands::register_info(app);
    vpdf::commands::register_merge(app);
    vpdf::commands::register_split(app);
    vpdf::commands::register_rotate(app);
    vpdf::commands::register_compress(app);
    vpdf::commands::register_encrypt(app);
    vpdf::commands::register_sign(app);
    vpdf::commands::register_validate(app);

    CLI11_PARSE(app, argc, argv);
    return 0;
}
