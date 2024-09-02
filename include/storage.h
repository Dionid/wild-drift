#include <vector>
#include "engine.h"

class NodesStorage {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        void AddNode(std::unique_ptr<Node> node) {
            this->nodes.push_back(node);
        }

        template <typename T>
        std::vector<Node*> GetByType() {
            std::vector<Node*> nodes;

            for (auto node: this->nodes) {
                if (dynamic_cast<T*>(node.get())) {
                    nodes.push_back(node.get());
                }
            }

            return nodes;
        }
};
