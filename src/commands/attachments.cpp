#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct AttachmentAddOptions {
    std::string input;
    std::string output;
    std::string file;
    std::string description;
};

struct AttachmentListOptions {
    std::string input;
};

struct AttachmentRemoveOptions {
    std::string input;
    std::string output;
    std::string name;
};

static int run_attachment_add(const AttachmentAddOptions& /*opts*/) {
    std::cerr << "Error: Attachment embedding is not yet supported by the vanillapdf library\n";
    return 1;
}

static int run_attachment_list(const AttachmentListOptions& /*opts*/) {
    std::cerr << "Error: Attachment listing is not yet supported by the vanillapdf library\n";
    return 1;
}

static int run_attachment_remove(const AttachmentRemoveOptions& /*opts*/) {
    std::cerr << "Error: Attachment removal is not yet supported by the vanillapdf library\n";
    return 1;
}

void register_attachments(CLI::App& app) {
    auto* cmd = app.add_subcommand("attachments", "Manage PDF file attachments");
    cmd->require_subcommand(1);

    // attachments add
    auto add_opts = std::make_shared<AttachmentAddOptions>();
    auto* add_cmd = cmd->add_subcommand("add", "Embed a file attachment in a PDF document");
    add_cmd->add_option("input", add_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    add_cmd->add_option("-o,--output", add_opts->output, "Output PDF file")
        ->required();
    add_cmd->add_option("-f,--file", add_opts->file, "File to attach")
        ->required()
        ->check(CLI::ExistingFile);
    add_cmd->add_option("-d,--description", add_opts->description, "Attachment description");

    add_cmd->callback([add_opts]() {
        try {
            std::exit(run_attachment_add(*add_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });

    // attachments list
    auto list_opts = std::make_shared<AttachmentListOptions>();
    auto* list_cmd = cmd->add_subcommand("list", "List file attachments in a PDF document");
    list_cmd->add_option("input", list_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);

    list_cmd->callback([list_opts]() {
        try {
            std::exit(run_attachment_list(*list_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });

    // attachments remove
    auto rm_opts = std::make_shared<AttachmentRemoveOptions>();
    auto* rm_cmd = cmd->add_subcommand("remove", "Remove a file attachment from a PDF document");
    rm_cmd->add_option("input", rm_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    rm_cmd->add_option("-o,--output", rm_opts->output, "Output PDF file")
        ->required();
    rm_cmd->add_option("-n,--name", rm_opts->name, "Attachment name to remove")
        ->required();

    rm_cmd->callback([rm_opts]() {
        try {
            std::exit(run_attachment_remove(*rm_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
