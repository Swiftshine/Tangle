#include "tangle.h"

enum args {
    EXECUTABLE = 0,
    INPUT_VERSION
};

int main() {
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

    std::vector<std::string> inputFiles = pfd::open_file("Select file(s)", ".", filters, pfd::opt::multiselect).result();

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
        if (std::string::npos == filepath.find(".gfa")) {
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
            fs::remove("error_log.txt");
        } else {
            pfd::message("Tangle", std::to_string(problems) + " file(s) could not be extracted. See error_log.txt for details.", pfd::choice::ok, pfd::icon::warning);
        }
        tangle::file_explorer(outputPath);
    } else {
        outputPath = pfd::save_file("Select output file", "", { "GfArch Files (*.gfa)", "*.gfa" }, pfd::opt::force_overwrite).result();

        if (outputPath.empty()) {
            // default behaviour is to use the first filename
            outputPath = nonArchives[0];
        }

        if (std::string::npos == outputPath.find(".gfa")) {
            outputPath += ".gfa";
        }


        // get version
        int version;
        std::cout << "Select an archive version. See README for details.\n";
        int compressionType = GfArch::CompressionType::BytePairEncoding;

        std::string input;
        while (true) {
            input = "";
            std::cin >> input;

            if ("2.0" == input) {
                version = GfArch::version::v2;
                break;
            } else if ("3.0" == input) {
                version = GfArch::version::v3;
                break;
            } else if ("3.1a" == input) {
                version = GfArch::version::v3_1;
                break;
            } else if ("3.1b" == input) {
                version = GfArch::version::v3_1;
                compressionType = GfArch::CompressionType::LZ77;
                break;
            }

            if (input.empty()) {
                // assume 3.0, BPE
                version = GfArch::version::v3;
                break;
            }

            std::cout << "Invalid input. Try again.\n";
        }
        
        tangle::reset_log();

        tangle::archive(nonArchives, outputPath, version, compressionType);

        pfd::message("Tangle", "Successfully archived file(s).", pfd::choice::ok, pfd::icon::info);

        tangle::file_explorer(fs::path(outputPath).parent_path().string());

        fs::remove("error_log.txt");
    }

    return EXIT_SUCCESS;
}