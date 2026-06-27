#include <glad/gl.h>

#include <filesystem>
#include <string_view>

#include "error.h"
#include "material.h"
#include "scenes/scenes.h"
#include "utilities.h"
#include "window.h"

namespace fs = std::filesystem;
using namespace LrnGL;

std::string GetAssetDirectory(std::string_view target = "assets")
{
    fs::path working_dir = fs::current_path();
    while (working_dir.has_parent_path())
    {
        auto enumerate_directories = fs::directory_iterator(working_dir);

        for (const fs::directory_entry& dir : enumerate_directories)
            if (dir.is_directory() && dir.path().filename() == target)
                return dir.path().string();

        working_dir = working_dir.parent_path();
    }

    ASSERT(false, "Failed to find working directory");
}

int main(int argc, const char** argv)
{
    SetAssetDirectory(GetAssetDirectory());
    Window window;

    glEnable(GL_DEPTH_TEST);

    LoadMaterialDefaults();

    // PhongMain(window, argc, argv);
    ModelLoadingMain(window, argc, argv);

    UnloadMaterialDefaults();
    return 0;
}
