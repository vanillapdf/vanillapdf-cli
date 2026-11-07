#include "commands/commands.h"
#include "utils/error.h"
#include "utils/page_range.h"

#include <vanillapdf/c_vanillapdf_api.h>

#include <CLI/CLI.hpp>

#include <iostream>
#include <map>
#include <string>

namespace {

const std::map<std::string, LoggingSeverity> LOG_LEVELS = {
    {"off",      LoggingSeverity_Off},
    {"critical", LoggingSeverity_Critical},
    {"error",    LoggingSeverity_Error},
    {"warning",  LoggingSeverity_Warning},
    {"info",     LoggingSeverity_Info},
    {"debug",    LoggingSeverity_Debug},
    {"trace",    LoggingSeverity_Trace},
};

const char* const COPYRIGHT_NOTICE =
    "Copyright 2026 Vanilla.PDF Labs\n"
    "Licensed under the Apache License, Version 2.0\n"
    "https://www.apache.org/licenses/LICENSE-2.0\n";

const char* const ABOUT_NOTICE =
    "vanillapdf-cli " VANILLAPDF_CLI_VERSION "\n"
    "The command-line toolkit for professional PDF processing,\n"
    "validation and digital signatures.\n"
    "\n"
    "Homepage:   https://vanillapdf.com\n"
    "Repository: https://github.com/vanillapdf/vanillapdf-cli\n"
    "Issues:     https://github.com/vanillapdf/vanillapdf-cli/issues\n"
    "Sponsor:    https://github.com/sponsors/vanillapdf\n"
    "\n"
    "Copyright 2026 Vanilla.PDF Labs - Apache License, Version 2.0\n";

} // namespace

int main(int argc, char** argv) {
    Logging_SetSeverity(LoggingSeverity_Off);

    CLI::App app("vanillapdf-cli - the command-line toolkit for professional PDF processing, "
        "validation and digital signatures");
    app.set_version_flag("--version", VANILLAPDF_CLI_VERSION);

    auto* copyright_flag = app.add_flag_callback("--copyright", []() {
        std::cout << COPYRIGHT_NOTICE;
        std::exit(vanillapdf::cli::VANILLAPDF_CLI_ERROR_SUCCESS);
    }, "Show copyright and license information");
    copyright_flag->trigger_on_parse();

    auto* about_flag = app.add_flag_callback("--about", []() {
        std::cout << ABOUT_NOTICE;
        std::exit(vanillapdf::cli::VANILLAPDF_CLI_ERROR_SUCCESS);
    }, "Show project information and links");
    about_flag->trigger_on_parse();

    app.require_subcommand(1);
    app.fallthrough();

    LoggingSeverity log_severity = LoggingSeverity_Off;
    app.add_option("--log-level", log_severity, "Set library log level (off, critical, error, warning, info, debug, trace)")
        ->transform(CLI::CheckedTransformer(LOG_LEVELS));
    app.add_flag("-v,--verbose", vanillapdf::cli::g_verbose, "Show detailed error information");

    app.parse_complete_callback([&log_severity]() {
        Logging_SetSeverity(log_severity);
    });

    int exit_code = vanillapdf::cli::VANILLAPDF_CLI_ERROR_SUCCESS;

    vanillapdf::cli::register_info(app, exit_code);
    vanillapdf::cli::register_metadata(app, exit_code);
    vanillapdf::cli::register_merge(app, exit_code);
    vanillapdf::cli::register_split(app, exit_code);
    vanillapdf::cli::register_pages(app, exit_code);
    vanillapdf::cli::register_rotate(app, exit_code);
    vanillapdf::cli::register_compress(app, exit_code);
    vanillapdf::cli::register_encrypt(app, exit_code);
    vanillapdf::cli::register_sign(app, exit_code);
    vanillapdf::cli::register_validate(app, exit_code);

    // With no arguments, show the full help rather than the terse
    // "A subcommand is required" error. Still exit non-zero, since no work was done.
    if (argc == 1) {
        std::cout << app.help();
        return vanillapdf::cli::VANILLAPDF_CLI_ERROR_FAILURE;
    }

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& ex) {
        // Also --help and --version, which exit with success
        return app.exit(ex);
    } catch (const vanillapdf::cli::PdfException& ex) {
        vanillapdf::cli::report_error(ex);
        return vanillapdf::cli::VANILLAPDF_CLI_ERROR_FAILURE;
    } catch (const vanillapdf::cli::PageRangeException& ex) {
        vanillapdf::cli::report_error(ex);
        return vanillapdf::cli::VANILLAPDF_CLI_ERROR_FAILURE;
    }

    return exit_code;
}
