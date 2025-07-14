#include <iostream>
#include "../Utils/ConnectionsBlock.h"

void print_connection(const ConnectionsBlock::StringConnection& conn) {
    std::cout << "Connection { "
              << "conn: " << conn.conn << ", "
              << "addr: " << conn.addr << ", "
              << "name: '" << conn.name << "' }"
              << std::endl;
}

void test_add_and_get() {
    std::cout << "=== Teste addConnection e getConnection ===" << std::endl;
    ConnectionsBlock cb;
    
    // Adicionando conexões
    cb.addConnection({101, 192168001, "Alice"});
    cb.addConnection({102, 192168002, "Bob"});
    cb.addConnection({103, 192168003, "Charlie"});
    
    // Buscando por socket
    std::cout << "Endereco de 102: " << cb.get_addr(102) << std::endl;
    std::cout << "Nome de 103: " << cb.get_name(103) << std::endl;
    
    // Buscando por nome
    auto conn = cb.getConnection("Bob");
    std::cout << "Conexao de Bob: ";
    print_connection(conn);
    
    std::cout << "Todos os nomes: " << cb.get_names() << std::endl;
}

void test_remove() {
    std::cout << "\n=== Teste removeConnection ===" << std::endl;
    ConnectionsBlock cb;
    
    cb.addConnection({201, 192168101, "David"});
    cb.addConnection({202, 192168102, "Eve"});
    cb.addConnection({203, 192168103, "Frank"});
    
    std::cout << "Antes de remover: " << cb.get_names() << std::endl;
    cb.removeConnection(202);
    std::cout << "Depois de remover 202: " << cb.get_names() << std::endl;
    
    // Tentando buscar conexão removida
    std::cout << "Nome de 202 (removido): " << cb.get_name(202) << std::endl;
}

void test_update() {
    std::cout << "\n=== Teste atualizacao de conexao ===" << std::endl;
    ConnectionsBlock cb;
    
    cb.addConnection({301, 192168201, "Grace"});
    cb.addConnection({301, 192168202, "Grace Hopper"}); // Atualiza
    
    auto conn = cb.getConnection("Grace Hopper");
    std::cout << "Conexao atualizada: ";
    print_connection(conn);
}

void test_collisions() {
    std::cout << "\n=== Teste de colisoes ===" << std::endl;
    ConnectionsBlock cb;
    
    // Esses valores podem colidir dependendo do tamanho da hash table
    cb.addConnection({401, 192168301, "Isaac"});
    cb.addConnection({401 + HASH_TABLE_SIZE, 192168302, "Newton"});
    
    std::cout << "Todas conexoes (deveria mostrar ambas): " << cb.get_names() << std::endl;
}

int main() {
    std::cout << "Iniciando testes do ConnectionsBlock\n";
    
    test_add_and_get();
    test_remove();
    test_update();
    test_collisions();
    
    std::cout << "\nTodos os testes concluidos!" << std::endl;
    return 0;
}