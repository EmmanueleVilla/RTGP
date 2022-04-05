class AABB {

    private:
    vector<AABB> children;
    void add_children(AABB& aabb) {
        children.push_back(aabb);
    }

    public:

    AABB(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) {
        MinX = minX;
        MaxX = maxX;
        MinY = minY;
        MaxY = maxY;
        MinZ = minZ;
        MaxZ = maxZ;
        IsLeaf = true;
        Hash = rand() % 10000;
    }

    AABB(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ, bool isLeaf) {
        MinX = minX;
        MaxX = maxX;
        MinY = minY;
        MaxY = maxY;
        MinZ = minZ;
        MaxZ = maxZ;
        IsLeaf = isLeaf;
        Hash = rand() % 10000;
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
        IsLeaf = true;
        Hash = rand() % 10000;
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
        IsLeaf = true;
        Hash = rand() % 10000;
    }

    /**
     * Initialize an AABB that occupies the whole map, with its children
     */
    AABB() {
        
        MinX = -20.0f;
        MaxX = 20.0f;
        MinY = 0.0f;
        MaxY = 12.0f;
        MinZ = -20.0f;
        MaxZ = 20.0f;
        IsLeaf = false;

        //--- FIRST QUARTER OF THE WHOLE AABB
        AABB firstQuarter = AABB(-20.0f, 0.0f, 0.0f, 12.0f,  -20.0f, 0.0f, false);

        //--- QUARTERS OF THE FIRST QUARTER
        AABB firstQuarterFirstQuarter = AABB(-20, -10, 0, 12, -20, -10, false);
        AABB firstQuarterSecondQuarter = AABB(-10, 0, 0, 12, -20, -10, false);
        AABB firstQuarterThirdQuarter = AABB(-20, -10, 0, 12, -10, 0, false);
        AABB firstQuarterForthQuarter = AABB(-10, 0, 0, 12, -10, 0, false);

        firstQuarter.add_children(firstQuarterFirstQuarter);
        firstQuarter.add_children(firstQuarterSecondQuarter);
        firstQuarter.add_children(firstQuarterThirdQuarter);
        firstQuarter.add_children(firstQuarterForthQuarter);

        //--- SECOND QUARTER OF THE WHOLE AABB
        AABB secondQuarter = AABB(0.0f, 20.0f, 0.0f, 12.0f,  0.0f, 20.0f, false);

        //--- QUARTERS OF THE SECOND QUARTER
        AABB secondQuarterFirstQuarter = AABB(0, 10, 0, 12, 0, 10, false);
        AABB secondQuarterSecondQuarter = AABB(0, 10, 0, 12, 10, 20, false);
        AABB secondQuarterThirdQuarter = AABB(10, 20, 0, 12, 0, 10, false);
        AABB secondQuarterForthQuarter = AABB(10, 20, 0, 12, 10, 20, false);

        secondQuarter.add_children(secondQuarterFirstQuarter);
        secondQuarter.add_children(secondQuarterSecondQuarter);
        secondQuarter.add_children(secondQuarterThirdQuarter);
        secondQuarter.add_children(secondQuarterForthQuarter);

        //--- THIRD QUARTER OF THE WHOLE AABB
        AABB thirdQuarter = AABB(-20.0f, 0.0f, 0.0f, 12.0f,  0.0f, 20.0f, false);

        //--- QUARTERS OF THE THIRD QUARTER
        AABB thirdQuarterFirstQuarter = AABB(-20, -10, 0, 12, 0, 10, false);
        AABB thirdQuarterSecondQuarter = AABB(-20, -10, 0, 12, 10, 20, false);
        AABB thirdQuarterThirdQuarter = AABB(-10, 0, 0, 12, 0, 10, false);
        AABB thirdQuarterForthQuarter = AABB(-10, 0, 0, 12, 10, 20, false);

        thirdQuarter.add_children(thirdQuarterFirstQuarter);
        thirdQuarter.add_children(thirdQuarterSecondQuarter);
        thirdQuarter.add_children(thirdQuarterThirdQuarter);
        thirdQuarter.add_children(thirdQuarterForthQuarter);
        
        //--- FORTH QUARTER OF THE WHOLE AABB
        AABB forthQuarter = AABB(0.0f, 20.0f, 0.0f, 12.0f,  -20.0f, 0.0f, false);

         //--- QUARTERS OF THE FORTH QUARTER
        AABB forthQuarterFirstQuarter = AABB(0, 10, 0, 12, -20, -10, false);
        AABB forthQuarterSecondQuarter = AABB(0, 10, 0, 12, -10, 0, false);
        AABB forthQuarterThirdQuarter = AABB(10, 20, 0, 12, -20, -10, false);
        AABB forthQuarterForthQuarter = AABB(10, 20, 0, 12, -10, 0, false);

        forthQuarter.add_children(forthQuarterFirstQuarter);
        forthQuarter.add_children(forthQuarterSecondQuarter);
        forthQuarter.add_children(forthQuarterThirdQuarter);
        forthQuarter.add_children(forthQuarterForthQuarter);

        add_children(firstQuarter);
        add_children(secondQuarter);
        add_children(thirdQuarter);
        add_children(forthQuarter);
        Hash = rand() % 10000;
    }

    float MinX = 9999;
    float MinY = 9999;
    float MinZ = 9999;
    float MaxX = -9999;
    float MaxY = -9999;
    float MaxZ = -9999;
    bool IsLeaf = false;
    int Hash;

    void addAABBToHierarchy(AABB& collider) {

        cout << "Try add aabb to " << this->toString() << endl;

        bool isLastLevel = false;

        //--- CHECK IF I AM THE LAST LEVEL IN THE AAB HIERARCHY
        if(children.size() == 0) {
            //--- I'M LAST LEVEL IF I DON'T HAVE CHILDER
            isLastLevel = true;
        } else {
            isLastLevel = false;
            //--- OR IF MY CHILDREN ARE LEAVES
            //--- I CHECK ONLY ONE BECAUSE IF ONE IS A LEAF, THEN ALL ARE LEAVES
            for (auto i= children.begin(); i!=children.end(); ++i) {
                AABB child = *i;
                if(child.IsLeaf) {
                    isLastLevel = true;
                }
            }
        }

        cout << "Am I the last level? " << isLastLevel << endl;

        //--- SINCE I'M THE LAST LEVEL, I ADD THIS AABB TO MY CHILDREN
        if(isLastLevel) {
            cout << "Child added to me: " << std::to_string(Hash) << endl;
            add_children(collider);

            cout << "---00000000000000000000---" << endl;
            cout << fullPrint(0) << endl;

            return;
        }

        //--- TRY COLLISION AGAINST EACH CHILD
        for (auto i= children.begin(); i!=children.end(); ++i) {
            AABB child = *i;
            cout << "Checking against " << child.toString() << endl;
            bool collisionX = (child.MinX <= collider.MaxX && child.MaxX >= collider.MinX);
            bool collisionZ = (child.MinZ <= collider.MaxZ && child.MaxZ >= collider.MinZ);
            if(collisionX && collisionZ) {
                cout << "COLLISION!" << endl;
                //--- I PASS THE AABB TO MY CHILDREN, IF THEY COLLIDE WITH IT
                child.addAABBToHierarchy(collider);
            }
        }
    }

    string toString() {
        return "[" + std::to_string(Hash) + "] X: { " + std::to_string(MinX) + "  " + std::to_string(MaxX) + " } Z: { " + std::to_string(MinZ) + "  " + std::to_string(MaxZ) + " } (" + std::to_string(children.size()) + " children)";
    }

    string fullPrint(int offset) {
        string output = this->toString();
        if(children.size() == 0) {
            return output;
        }
        for (auto i=children.begin(); i!=children.end(); ++i) {
            output += "\n";
            for(int index = 0; index <= offset; index++) {
                output += "   ";
            }
            output += (*i).fullPrint(offset + 1);
        }
        return output;
    }
};