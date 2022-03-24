#include <glad/glad.h>

class Utilities {
    public:
    Utilities() {
    }

    vector<GLfloat> getVertices(glm::vec3 pos, GLfloat scale, GLfloat dx, GLfloat dy, GLfloat dz) {
        vector<GLfloat> output;

        output.push_back(pos.x + scale - dx);
        output.push_back(dy * scale);
        output.push_back(pos.z - scale - dz);

        output.push_back(pos.x + scale - dx);
        output.push_back(0);
        output.push_back(pos.z - scale - dz);

        output.push_back(pos.x - scale - dx);
        output.push_back(0);
        output.push_back(pos.z - scale - dz);

        output.push_back(pos.x - scale - dx);
        output.push_back(dy * scale);
        output.push_back(pos.z - scale - dz);

        output.push_back(pos.x + scale - dx);
        output.push_back(dy * scale);
        output.push_back(pos.z + scale - dz);

        output.push_back(pos.x + scale - dx);
        output.push_back(0);
        output.push_back(pos.z + scale - dz);

        output.push_back(pos.x - scale - dx);
        output.push_back(0);
        output.push_back(pos.z + scale - dz);

        output.push_back(pos.x - scale - dx);
        output.push_back(dy * scale);
        output.push_back(pos.z + scale - dz);

        return output;
    };
};