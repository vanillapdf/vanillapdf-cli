#ifndef VANILLAPDF_CLI_COMMANDS_COMMANDS_HPP
#define VANILLAPDF_CLI_COMMANDS_COMMANDS_HPP

namespace CLI { class App; }

namespace vpdf::commands {

void register_info(CLI::App& app);
void register_merge(CLI::App& app);
void register_split(CLI::App& app);
void register_rotate(CLI::App& app);
void register_compress(CLI::App& app);
void register_encrypt(CLI::App& app);
void register_sign(CLI::App& app);
void register_validate(CLI::App& app);

} // namespace vpdf::commands

#endif // VANILLAPDF_CLI_COMMANDS_COMMANDS_HPP
