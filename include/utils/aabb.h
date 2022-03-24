class AABB {
    public:

    AABB(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) {
        MinX = minX;
        MaxX = maxX;
        MinY = minY;
        MaxY = maxY;
        MinZ = minZ;
        MaxZ = maxZ;
    }

    AABB(vector<GLfloat> vertices) {
        int index = 0;
        for (auto i=vertices.begin(); i!=vertices.end(); ++i) {
            float v = *i;
            if(index % 3 == 0) {
                if(v < MinX) {
                    MinX = v;
                }
                if(v > MaxX) {
                    MaxX = v;
                }
            }
            if(index % 3 == 1) {
                if(v < MinY) {
                    MinY = v;
                }
                if(v > MaxY) {
                    MaxY = v;
                }
            }
            if(index % 3 == 2) {
                if(v < MinZ) {
                    MinZ = v;
                }
                if(v > MaxZ) {
                    MaxZ = v;
                }
            }
            index++;
        }
    }

    AABB(GLfloat vertices[]) {
        for (int i = 0; i < 24; i++) {
            float v = vertices[i];
            if(i % 3 == 0) {
                if(v < MinX) {
                    MinX = v;
                }
                if(v > MaxX) {
                    MaxX = v;
                }
            }
            if(i % 3 == 1) {
                if(v < MinY) {
                    MinY = v;
                }
                if(v > MaxY) {
                    MaxY = v;
                }
            }
            if(i % 3 == 2) {
                if(v < MinZ) {
                    MinZ = v;
                }
                if(v > MaxZ) {
                    MaxZ = v;
                }
            }
        }
    }

    float MinX = 9999;
    float MinY = 9999;
    float MinZ = 9999;
    float MaxX = -9999;
    float MaxY = -9999;
    float MaxZ = -9999;
};

class AABBNode : public AABB {

    private:
    vector<AABB> children;
    
    public:
    AABBNode(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) : AABB(minX, maxX, minY, maxY, minZ, maxZ) {

    }

    void add_children(AABB aabb) {
        children.push_back(aabb);
    }
};