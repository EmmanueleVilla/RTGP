//---  Std. Includes
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
//---  Loader estensions OpenGL
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <chrono>

#include <glad/glad.h>

//---  GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

//---  confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

class CsvLoader {
    public:
    vector<vector<string>> read(string path) {
        vector<vector<string>> content;
        vector<string> row;
        string line, word;
        fstream file (path, ios::in);
        if(file.is_open()) {
            while(getline(file, line))
            {
                row.clear();

                stringstream str(line);

                while(getline(str, word, ',')) {
                    row.push_back(word);
                }
                content.push_back(row);
            }
        } else {
            std::cout << "Failed to load level data" << std::endl;
            exit(0);
        }
        return content;
    }
};