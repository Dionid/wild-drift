#ifndef CENGINE_STORAGE_H_
#define CENGINE_STORAGE_H_

#include <vector>
#include "node.h"

class NodeStorage {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        void AddNode(std::unique_ptr<Node> node) {
            this->nodes.push_back(std::move(node));
        }

        template <typename T>
        std::vector<Node*> GetByType() {
            std::vector<Node*> nodes;

            for (const auto& node: this->nodes) {
                if (dynamic_cast<T*>(node.get())) {
                    nodes.push_back(node.get());
                }
            }

            return nodes;
        }

        template <typename T>
        Node* GetFirstByType() {
            for (const auto& node: this->nodes) {
                if (dynamic_cast<T*>(node.get())) {
                    return node.get();
                }
            }

            return nullptr;
        }
};

#endif // CENGINE_STORAGE_H_