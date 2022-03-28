class VerticesBuilder {
    public:

    VerticesBuilder() {};

    vector<GLfloat> build(glm::vec3 pos, GLfloat dy, glm::vec3 size) {
        vector<GLfloat> vertices;
        vertices.push_back(pos.x + size.x);
        vertices.push_back(dy);
        vertices.push_back(pos.z - size.z);

        vertices.push_back(pos.x + size.x);
        vertices.push_back(0);
        vertices.push_back(pos.z - size.z);

        vertices.push_back(pos.x - size.x);
        vertices.push_back(0);
        vertices.push_back(pos.z - size.z);

        vertices.push_back(pos.x - size.x);
        vertices.push_back(dy);
        vertices.push_back(pos.z - size.z);

        vertices.push_back(pos.x + size.x);
        vertices.push_back(dy);
        vertices.push_back(pos.z + size.z);

        vertices.push_back(pos.x + size.x);
        vertices.push_back(0);
        vertices.push_back(pos.z + size.z);

        vertices.push_back(pos.x - size.x);
        vertices.push_back(0);
        vertices.push_back(pos.z + size.z);

        vertices.push_back(pos.x - size.x);
        vertices.push_back(dy);
        vertices.push_back(pos.z + size.z);

        return vertices;
    }

};