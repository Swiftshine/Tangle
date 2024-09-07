#include "tangle.h"

enum args {
    EXECUTABLE = 0,
    INPUT_VERSION
};

int main(int argc, char *argv[]) {
    // check that a pfd backend is available
    if (!pfd::settings::available()) {
        std::cerr << "portable-file-dialogs is not available on this platform.\n";
        return EXIT_FAILURE;
    }

    // get files that user wants to extract

    std::vector<std::string> filters = {
        "GfArch Files (*.gfa)", "*.gfa",
        "All Files (*.*)", "*.*"
    };

    std::vector<std::string> inputFiles = pfd::open_file("Select GfArch file(s)", ".", filters, pfd::opt::multiselect).result();

    if (inputFiles.empty()) {
        // user cancelled
        return EXIT_SUCCESS;
    }

    // sort files alphabetically
    std::sort(inputFiles.begin(), inputFiles.end());

    // determine if the user wants to extract or archive files
    // in order to do this, go through every file and see if it ends in .gfa
    // if it doesn't, then we archive it and discard archive files from the list

    std::vector<std::string> nonArchives;

    for (std::string& filepath : inputFiles) {
        if (std::string::npos != filepath.find(".gfa")) {
            nonArchives.push_back(filepath);
        }
    }

    bool doExtract = nonArchives.empty();

    std::string outputPath;


    if (doExtract) {
        outputPath = pfd::select_folder("Select output folder").result();

        if (outputPath.empty()) {
            // use current directory
            outputPath = ".";
        }

        tangle::reset_log();

        int problems = tangle::extract(inputFiles, outputPath);

        if (0 == problems) {
            pfd::message("Tangle", "Successfully extracted file(s).", pfd::choice::ok, pfd::icon::info);
        } else {
            pfd::message("Tangle", std::to_string(problems) + " file(s) could not be extracted. See error_log.txt for details.", pfd::choice::ok, pfd::icon::warning);
        }

        tangle::file_explorer(outputPath);
    } else {
        outputPath = pfd::save_file("Select output file", "", { "GfArch Files (*.gfa)", "*.gfa" }, pfd::opt::force_overwrite).result();

        if (outputPath.empty()) {
            // default behaviour is to use the first filename
            outputPath = inputFiles[0];
        }

        tangle::reset_log();


        // GfArch 3.0 will be the default
        int version = GfArch::version::v3;

        if (2 > argc) {
            std::string inputVersion = argv[INPUT_VERSION];

            if ("2.0" == inputVersion) {
                version = GfArch::version::v2;
            } else if ("3.0" == inputVersion) {
                version = GfArch::version::v3;
            } else if ("3.1" == inputVersion) {
                version = GfArch::version::v3_1;
            }
        }

        tangle::archive(nonArchives, outputPath, version);
    }

    return EXIT_SUCCESS;
}