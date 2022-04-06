class AABB {

    private:
    vector<AABB> children;
    void add_children(AABB& aabb) {
        children.push_back(aabb);
    }

    public:

    void initWithValues(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ, bool isLeaf, bool acceptChildren) {
        MinX = minX;
        MaxX = maxX;
        MinY = minY;
        MaxY = maxY;
        MinZ = minZ;
        MaxZ = maxZ;
        IsLeaf = isLeaf;
        AcceptChildren = acceptChildren;
        Hash = rand() % 10000;
    }

    AABB(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) {
        initWithValues(minX, maxX, minY, maxY, minZ, maxZ, false, false); 
    }

    AABB(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ, bool isLeaf) {
        initWithValues(minX, maxX, minY, maxY, minZ, maxZ, isLeaf, false); 
    }

    AABB(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ, bool isLeaf, bool acceptChildren) {
        initWithValues(minX, maxX, minY, maxY, minZ, maxZ, isLeaf, acceptChildren); 
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
        
        int side = 64;
        int half = side / 2;

        MinX = -side;
        MaxX = side;
        MinY = 0;
        MaxY = 12;
        MinZ = -side;
        MaxZ = side;
        IsLeaf = false;
        AcceptChildren = false;
        Hash = rand() % 10000;

        //--- FIRST QUARTER OF THE WHOLE AABB
        AABB firstQuarter = AABB(-side, 0, 0, 12, -side, 0, false);

        //--- QUARTERS OF THE FIRST QUARTER
        AABB firstQuarterFirstQuarter = AABB(-side, -half, 0, 12, -side, -half, false, true);
        AABB firstQuarterSecondQuarter = AABB(-half, 0, 0, 12, -side, -half, false, true);
        AABB firstQuarterThirdQuarter = AABB(-side, -half, 0, 12, -half, 0, false, true);
        AABB firstQuarterForthQuarter = AABB(-half, 0, 0, 12, -half, 0, false, true);

        firstQuarter.add_children(firstQuarterFirstQuarter);
        firstQuarter.add_children(firstQuarterSecondQuarter);
        firstQuarter.add_children(firstQuarterThirdQuarter);
        firstQuarter.add_children(firstQuarterForthQuarter);

        //--- SECOND QUARTER OF THE WHOLE AABB
        AABB secondQuarter = AABB(0, side, 0, 12,  0, side, false);

        //--- QUARTERS OF THE SECOND QUARTER
        AABB secondQuarterFirstQuarter = AABB(0, half, 0, 12, 0, half, false, true);
        AABB secondQuarterSecondQuarter = AABB(0, half, 0, 12, half, side, false, true);
        AABB secondQuarterThirdQuarter = AABB(half, side, 0, 12, 0, half, false, true);
        AABB secondQuarterForthQuarter = AABB(half, side, 0, 12, half, side, false, true);

        secondQuarter.add_children(secondQuarterFirstQuarter);
        secondQuarter.add_children(secondQuarterSecondQuarter);
        secondQuarter.add_children(secondQuarterThirdQuarter);
        secondQuarter.add_children(secondQuarterForthQuarter);

        //--- THIRD QUARTER OF THE WHOLE AABB
        AABB thirdQuarter = AABB(-side, 0, 0, 12,  0, side, false);

        //--- QUARTERS OF THE THIRD QUARTER
        AABB thirdQuarterFirstQuarter = AABB(-side, -half, 0, 12, 0, half, false, true);
        AABB thirdQuarterSecondQuarter = AABB(-side, -half, 0, 12, half, side, false, true);
        AABB thirdQuarterThirdQuarter = AABB(-half, 0, 0, 12, 0, half, false, true);
        AABB thirdQuarterForthQuarter = AABB(-half, 0, 0, 12, half, side, false, true);

        thirdQuarter.add_children(thirdQuarterFirstQuarter);
        thirdQuarter.add_children(thirdQuarterSecondQuarter);
        thirdQuarter.add_children(thirdQuarterThirdQuarter);
        thirdQuarter.add_children(thirdQuarterForthQuarter);
        
        //--- FORTH QUARTER OF THE WHOLE AABB
        AABB forthQuarter = AABB(0, side, 0, 12,  -side, 0, false);

         //--- QUARTERS OF THE FORTH QUARTER
        AABB forthQuarterFirstQuarter = AABB(0, half, 0, 12, -side, -half, false, true);
        AABB forthQuarterSecondQuarter = AABB(0, half, 0, 12, -half, 0, false, true);
        AABB forthQuarterThirdQuarter = AABB(half, side, 0, 12, -side, -half, false, true);
        AABB forthQuarterForthQuarter = AABB(half, side, 0, 12, -half, 0, false, true);

        forthQuarter.add_children(forthQuarterFirstQuarter);
        forthQuarter.add_children(forthQuarterSecondQuarter);
        forthQuarter.add_children(forthQuarterThirdQuarter);
        forthQuarter.add_children(forthQuarterForthQuarter);

        add_children(firstQuarter);
        add_children(secondQuarter);
        add_children(thirdQuarter);
        add_children(forthQuarter);
    }

    float MinX = 9999;
    float MinY = 9999;
    float MinZ = 9999;
    float MaxX = -9999;
    float MaxY = -9999;
    float MaxZ = -9999;
    bool IsLeaf = false;
    bool AcceptChildren = false;
    int Hash;

    //--- OPTIMIZED COLLISION BETWEEN 30 AND 75 MICROSECONDS
    bool checkXZCollision(AABB& collider) {
        //--- TRY COLLISION AGAINST ME
        bool collisionX = (MinX <= collider.MaxX && MaxX >= collider.MinX);
        
        if(collisionX) {
            bool collisionZ = (MinZ <= collider.MaxZ && MaxZ >= collider.MinZ);
            //--- COLLIDES AGAINST ME
            if(collisionZ) {
                //--- IF I'M A LEAF, RETURN TRUE
                if(IsLeaf) {
                    return true;
                }
                if(children.size() == 0) {
                    return false;
                }
                //--- ELSE, PASS THE CHECK TO MY CHILDREN
                for(AABB& child : children) {
                    bool childCollision = child.checkXZCollision(collider);
                    if(childCollision) {
                        return true;
                    }
                }
                return false;
            } else {
                return false;
            }
            
        }
        return false;
    }

    void addAABBToHierarchy(AABB& collider) {

        bool isLastLevel = false;

        //--- SINCE I'M THE LAST LEVEL, I ADD THIS AABB TO MY CHILDREN
        if(AcceptChildren) {
            add_children(collider);
            return;
        }

        //--- TRY COLLISION AGAINST EACH CHILD
        for(AABB& child : children) {
            bool collisionX = (child.MinX <= collider.MaxX && child.MaxX >= collider.MinX);
            if(collisionX) {
                bool collisionZ = (child.MinZ <= collider.MaxZ && child.MaxZ >= collider.MinZ);
                //--- I PASS THE AABB TO MY CHILDREN, IF THEY COLLIDE WITH IT
                if(collisionZ) {
                    child.addAABBToHierarchy(collider);
                }
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