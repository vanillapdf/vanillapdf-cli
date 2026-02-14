#include <CLI/CLI.hpp>
#include <iostream>

#include "commands/commands.hpp"

int main(int argc, char** argv) {
    CLI::App app{"vanillapdf-cli - PDF manipulation tool"};
    app.set_version_flag("--version", VANILLAPDF_CLI_VERSION);
    app.require_subcommand(1);

    vpdf::commands::register_info(app);
    vpdf::commands::register_merge(app);
    vpdf::commands::register_split(app);
    vpdf::commands::register_compress(app);
    vpdf::commands::register_encrypt(app);
    vpdf::commands::register_sign(app);

    CLI11_PARSE(app, argc, argv);
    return 0;
}
