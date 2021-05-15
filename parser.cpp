#include <memory>
#include <unordered_set>
#include "sjp.h"

namespace sjp {

ast parse_string(const char* str) {
    auto* program = souffle::ProgramFactory::newInstance("sjp");
    assert(program != nullptr);
    auto& symbol_table = program->getSymbolTable();
    auto& record_table = program->getRecordTable();
    /* relations */
    auto* token_rel = program->getRelation("token");
    auto* num_tokens_rel = program->getRelation("num_tokens");
    auto* token_type_rel = program->getRelation("token_type");
    auto* parent_of_rel = program->getRelation("parent_of");
    auto* parent_of_list_rel = program->getRelation("parent_of_list");
    auto* root_rel = program->getRelation("root");
    /* input */
    auto [tokens, token_type] = lex(str);
    for (const auto& [content, start, end] : tokens) {
        token_rel->insert(souffle::tuple(token_rel, {symbol_table.encode(content), int32_t(start), int32_t(end)}));
    }
    for (const auto& [id, type] : token_type) {
        token_type_rel->insert(souffle::tuple(token_type_rel, {int32_t(id), symbol_table.encode(type)}));
    }
    num_tokens_rel->insert(souffle::tuple(num_tokens_rel,
            {int32_t(tokens.size())}));
    /* run program */
    program->run();
    /* output */
    ast tree;
    std::unordered_set<ast_node> all_ast_nodes;
    for (auto& output : *parent_of_rel) {
        ast_node parent;
        std::string symbol;
        ast_node child;
        output >> parent >> symbol >> child;
        assert(parent != 0);
        all_ast_nodes.emplace(parent);
        all_ast_nodes.emplace(child);
        tree.parent_of[parent].emplace_back(std::move(symbol), child);
    }
    for (auto& output : *parent_of_list_rel) {
        ast_node parent;
        std::string symbol;
        int list;
        output >> parent >> symbol >> list;
        assert(parent != 0);
        all_ast_nodes.emplace(parent);
        std::vector<ast_node> result;
        while (list != 0) {
            const auto* record = record_table.unpack(list, 2);
            result.push_back(record[0]);
            all_ast_nodes.emplace(record[0]);
            list = record[1];
        }
        tree.parent_of_list[parent].emplace_back(std::move(symbol), std::move(result));
    }
    for (auto& output : *root_rel) {
        output >> tree.root;
        all_ast_nodes.emplace(tree.root);
    }
    for (auto node : all_ast_nodes) {
        if (node == 0) continue;
        const auto* record = record_table.unpack(node, 3);
        tree.name[node] = symbol_table.decode(record[0]);
        tree.starts_at[node] = record[1];
        tree.ends_at[node] = record[2];
    }
    delete program;
    return tree;
}


ast parse_file(const char* filename) {
    std::ifstream t(filename);
    return parse_string(std::string((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>()).c_str());
}

}
