#include "tangle.h"

int main() {
    // check that a pfd backend is available
    if (!pfd::settings::available()) {
        std::cerr << "Portable File Dialogs is not available on this platform.\n";
        return EXIT_FAILURE;
    }

    // get files that user wants to extract

    std::vector<std::string> filters = {
        "GfArch Files (*.gfa)", "*.gfa" 
    };

    std::vector<std::string> inputFiles = pfd::open_file("Select GfArch file(s)", ".", filters, pfd::opt::multiselect).result();

    if (inputFiles.empty()) {
        return EXIT_SUCCESS;
    }

    // select a folder to extract files to
    std::string outputFolder = pfd::select_folder("Select output folder").result();
    
    if (outputFolder.empty()) {
        // you can just put it in the cwd
        outputFolder = ".";
    }

    // before beginning, clear error log

    std::ofstream logFile("error_log.txt");
    logFile.close();

    // re-archiving files is not supported at the moment

    int problems = tangle::extract(inputFiles, outputFolder);
    if (0 == problems) {
        pfd::message("Tangle", "Successfully extracted file(s).", pfd::choice::ok, pfd::icon::info);
    } else {
        pfd::message("Tangle", std::to_string(problems) + " file(s) could not be extracted. See error_log.txt for details.", pfd::choice::ok, pfd::icon::warning);
    }

    tangle::file_explorer(outputFolder);

    return EXIT_SUCCESS;
}