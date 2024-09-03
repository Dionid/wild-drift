#include "cengine/cengine.h"
#include "entity.h"

class MainLevel {
    public:
        int screenWidth;
        int screenHeight;
        Scene* scene;

        MainLevel(
            const int screenWidth,
            const int screenHeight,
            Scene* scene
        );
        void InitMainLevel(); 
};