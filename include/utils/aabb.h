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
    void add_children(AABB aabb) {
        children.push_back(aabb);
    }
    
    public:
    AABBNode(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) : AABB(minX, maxX, minY, maxY, minZ, maxZ) {

    }

    /**
     * Initialize an AABB that occupies the whole map, with its children
     */
    AABBNode() : AABB(-20.0f, 20.0f, 0.0f, 12.0f, -20.0f, 20.0f) {
            
        //--- FIRST QUARTER OF THE WHOLE AABB
        AABBNode firstQuarter = AABBNode(-20.0f, 0.0f, 0.0f, 12.0f,  -20.0f, 0.0f);

        //--- QUARTERS OF THE FIRST QUARTER
        AABBNode firstQuarterFirstQuarter = AABBNode(-20, -10, 0, 12, -20, -10);
        AABBNode firstQuarterSecondQuarter = AABBNode(-10, 0, 0, 12, -20, -10);
        AABBNode firstQuarterThirdQuarter = AABBNode(-20, -10, 0, 12, -10, 0);
        AABBNode firstQuarterForthQuarter = AABBNode(-10, 0, 0, 12, -10, 0);

        firstQuarter.add_children(firstQuarterFirstQuarter);
        firstQuarter.add_children(firstQuarterSecondQuarter);
        firstQuarter.add_children(firstQuarterThirdQuarter);
        firstQuarter.add_children(firstQuarterForthQuarter);

        //--- SECOND QUARTER OF THE WHOLE AABB
        AABBNode secondQuarter = AABBNode(0.0f, 20.0f, 0.0f, 12.0f,  0.0f, 20.0f);

        //--- QUARTERS OF THE SECOND QUARTER
        AABBNode secondQuarterFirstQuarter = AABBNode(0, 10, 0, 12, 0, 10);
        AABBNode secondQuarterSecondQuarter = AABBNode(0, 10, 0, 12, 10, 20);
        AABBNode secondQuarterThirdQuarter = AABBNode(10, 20, 0, 12, 0, 10);
        AABBNode secondQuarterForthQuarter = AABBNode(10, 20, 0, 12, 10, 20);

        secondQuarter.add_children(secondQuarterFirstQuarter);
        secondQuarter.add_children(secondQuarterSecondQuarter);
        secondQuarter.add_children(secondQuarterThirdQuarter);
        secondQuarter.add_children(secondQuarterForthQuarter);

        //--- THIRD QUARTER OF THE WHOLE AABB
        AABBNode thirdQuarter = AABBNode(-20.0f, 0.0f, 0.0f, 12.0f,  0.0f, 20.0f);

        //--- QUARTERS OF THE THIRD QUARTER
        AABBNode thirdQuarterFirstQuarter = AABBNode(-20, -10, 0, 12, 0, 10);
        AABBNode thirdQuarterSecondQuarter = AABBNode(-20, -10, 0, 12, 10, 20);
        AABBNode thirdQuarterThirdQuarter = AABBNode(-10, 0, 0, 12, 0, 10);
        AABBNode thirdQuarterForthQuarter = AABBNode(-10, 0, 0, 12, 10, 20);

        thirdQuarter.add_children(thirdQuarterFirstQuarter);
        thirdQuarter.add_children(thirdQuarterSecondQuarter);
        thirdQuarter.add_children(thirdQuarterThirdQuarter);
        thirdQuarter.add_children(thirdQuarterForthQuarter);
        
        //--- FORTH QUARTER OF THE WHOLE AABB
        AABBNode forthQuarter = AABBNode(0.0f, 20.0f, 0.0f, 12.0f,  -20.0f, 0.0f);

         //--- QUARTERS OF THE FORTH QUARTER
        AABBNode forthQuarterFirstQuarter = AABBNode(0, 10, 0, 12, -20, -10);
        AABBNode forthQuarterSecondQuarter = AABBNode(0, 10, 0, 12, -10, 0);
        AABBNode forthQuarterThirdQuarter = AABBNode(10, 20, 0, 12, -20, -10);
        AABBNode forthQuarterForthQuarter = AABBNode(10, 20, 0, 12, -10, 0);

        forthQuarter.add_children(forthQuarterFirstQuarter);
        forthQuarter.add_children(forthQuarterSecondQuarter);
        forthQuarter.add_children(forthQuarterThirdQuarter);
        forthQuarter.add_children(forthQuarterForthQuarter);

        add_children(firstQuarter);
        add_children(secondQuarter);
        add_children(thirdQuarter);
        add_children(forthQuarter);
    }
};