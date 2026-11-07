#ifndef _VANILLAPDF_CLI_COMMANDS_H
#define _VANILLAPDF_CLI_COMMANDS_H

namespace CLI {
class App;
} // CLI

namespace vanillapdf {
namespace cli {

constexpr int VANILLAPDF_CLI_ERROR_SUCCESS = 0;
constexpr int VANILLAPDF_CLI_ERROR_FAILURE = 1;

// Each command registers its CLI11 subcommand with the application. The
// subcommand callback runs the command and stores its result in exit_code,
// which main returns once parsing has finished. Errors are thrown as
// PdfException or PageRangeException and reported by main.
void register_info(CLI::App& app, int& exit_code);
void register_metadata(CLI::App& app, int& exit_code);
void register_merge(CLI::App& app, int& exit_code);
void register_split(CLI::App& app, int& exit_code);
void register_pages(CLI::App& app, int& exit_code);
void register_rotate(CLI::App& app, int& exit_code);
void register_compress(CLI::App& app, int& exit_code);
void register_encrypt(CLI::App& app, int& exit_code);
void register_sign(CLI::App& app, int& exit_code);
void register_validate(CLI::App& app, int& exit_code);

} // cli
} // vanillapdf

#endif /* _VANILLAPDF_CLI_COMMANDS_H */
