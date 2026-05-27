// Leonardo Merino Villela - 2189723
// Mateus Domingos - 2189411
// Paulo César da Silva Dorazzi Branco - 2196455
// Joao Victor Garcia Gregorio - 2241389
// Trabalho 2 - Algoritmos em Grafos

#include <algorithm>
#include <cctype>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct Aresta {
    string destino;
    int peso;
};

using ListaAdjacencia = map<string, vector<Aresta>>;

string removerEspacosDasPontas(const string& texto) { // Remove espacos em branco no inicio e no fim de uma string.
    size_t inicio = 0;
    while (inicio < texto.size() && isspace(static_cast<unsigned char>(texto[inicio]))) {
        ++inicio;
    }

    size_t fim = texto.size();
    while (fim > inicio && isspace(static_cast<unsigned char>(texto[fim - 1]))) {
        --fim;
    }

    return texto.substr(inicio, fim - inicio);
}

// Divide linhas no formato "origem;destino;" ou "origem;destino;peso;".
vector<string> separarPorPontoEVirgula(const string& linha) { // Separa a linha por ';' e retorna apenas campos nao vazios.
    vector<string> partes;
    string parte;
    stringstream ss(linha);

    while (getline(ss, parte, ';')) {
        parte = removerEspacosDasPontas(parte);
        if (!parte.empty()) {
            partes.push_back(parte);
        }
    }

    return partes;
}

void adicionarVerticeSeNecessario(ListaAdjacencia& grafo, const string& vertice) { // Garante que o vertice exista no mapa de adjacencia.
    if (grafo.find(vertice) == grafo.end()) {
        grafo[vertice] = vector<Aresta>();
    }
}

void adicionarAresta(ListaAdjacencia& grafo, const string& origem, const string& destino, int peso) { // Insere uma aresta origem -> destino com peso.
    adicionarVerticeSeNecessario(grafo, origem);
    adicionarVerticeSeNecessario(grafo, destino);
    grafo[origem].push_back({destino, peso});
}

void ordenarListas(ListaAdjacencia& grafo) { // Ordena vizinhos para garantir comportamento deterministico.
    for (auto& item : grafo) {
        sort(item.second.begin(), item.second.end(), [](const Aresta& a, const Aresta& b) {
            if (a.destino != b.destino) {
                return a.destino < b.destino;
            }
            return a.peso < b.peso;
        });
    }
}

// Le grafo sem peso. Quando dirigido == false, cada aresta e registrada nos dois sentidos.
ListaAdjacencia lerGrafoNaoPonderado(const string& nomeArquivo, bool dirigido) { // Le arquivo sem peso e monta o grafo em memoria.
    ifstream arquivo(nomeArquivo); // Abre o arquivo de entrada.
    if (!arquivo.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo " + nomeArquivo);
    }

    ListaAdjacencia grafo;
    string linha;

    while (getline(arquivo, linha)) { // Processa o arquivo linha a linha.
        vector<string> campos = separarPorPontoEVirgula(linha); // Extrai origem e destino da linha.
        if (campos.size() < 2) {
            continue;
        }

        adicionarAresta(grafo, campos[0], campos[1], 1); // Grafo nao ponderado usa peso padrao 1.
        if (!dirigido) { // Se nao for dirigido, adiciona tambem a aresta reversa.
            adicionarAresta(grafo, campos[1], campos[0], 1);
        }
    }

    ordenarListas(grafo); // Ordena os vizinhos para manter precedencia lexicografica.
    return grafo;
}

// Le grafo ponderado usando o terceiro campo da linha como custo da aresta.
ListaAdjacencia lerGrafoPonderado(const string& nomeArquivo, bool dirigido) { // Le arquivo com peso e monta o grafo em memoria.
    ifstream arquivo(nomeArquivo); // Abre o arquivo de entrada.
    if (!arquivo.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo " + nomeArquivo);
    }

    ListaAdjacencia grafo;
    string linha;

    while (getline(arquivo, linha)) { // Processa o arquivo linha a linha.
        vector<string> campos = separarPorPontoEVirgula(linha); // Extrai origem, destino e peso da linha.
        if (campos.size() < 3) {
            continue;
        }

        int peso = stoi(campos[2]); // Converte o peso textual para inteiro.
        adicionarAresta(grafo, campos[0], campos[1], peso);
        if (!dirigido) { // Se nao for dirigido, adiciona tambem a aresta reversa.
            adicionarAresta(grafo, campos[1], campos[0], peso);
        }
    }

    ordenarListas(grafo); // Ordena os vizinhos para manter precedencia lexicografica.
    return grafo;
}

void imprimirListaAdjacencia(const ListaAdjacencia& grafo, bool ponderado) { // Mostra cada vertice e sua lista de vizinhos.
    for (const auto& item : grafo) {
        cout << item.first << ": ";

        for (size_t i = 0; i < item.second.size(); ++i) {
            const Aresta& aresta = item.second[i];
            cout << aresta.destino;
            if (ponderado) {
                cout << "(" << aresta.peso << ")";
            }
            if (i + 1 < item.second.size()) {
                cout << ", ";
            }
        }

        cout << '\n';
    }
}

// BFS por camadas: vertices com a mesma distancia sao visitados em ordem lexicografica.
vector<string> buscaEmLargura(const ListaAdjacencia& grafo, const string& inicio) { // Executa BFS partindo de um vertice inicial.
    vector<string> ordem;
    set<string> visitados;

    if (grafo.find(inicio) == grafo.end()) { // Se o vertice inicial nao existir, retorna vazio.
        return ordem;
    }

    visitados.insert(inicio);
    vector<string> camadaAtual;
    camadaAtual.push_back(inicio);

    while (!camadaAtual.empty()) { // Processa o grafo por camadas de distancia.
        sort(camadaAtual.begin(), camadaAtual.end()); // Ordena a camada para respeitar desempate lexicografico.
        set<string> proximaCamada; // Conjunto para acumular a proxima camada sem repeticoes.

        for (const string& atual : camadaAtual) {
            ordem.push_back(atual);

            for (const Aresta& aresta : grafo.at(atual)) { // Percorre os vizinhos do vertice atual.
                if (visitados.find(aresta.destino) == visitados.end()) {
                    visitados.insert(aresta.destino); // Marca o vertice para evitar revisitas.
                    proximaCamada.insert(aresta.destino); // Agenda vertice para a camada seguinte.
                }
            }
        }

        camadaAtual.assign(proximaCamada.begin(), proximaCamada.end()); // Troca para a proxima camada.
    }

    return ordem;
}

// DFS recursiva seguindo a lista de adjacencia previamente ordenada.
void dfsRecursiva(const ListaAdjacencia& grafo, const string& atual, set<string>& visitados, vector<string>& ordem) { // Parte recursiva da DFS.
    visitados.insert(atual); // Marca o vertice atual como visitado.
    ordem.push_back(atual); // Registra a ordem de visita.

    auto it = grafo.find(atual); // Busca os vizinhos do vertice atual.
    if (it == grafo.end()) {
        return;
    }

    for (const Aresta& aresta : it->second) { // Tenta aprofundar em cada vizinho.
        if (visitados.find(aresta.destino) == visitados.end()) {
            dfsRecursiva(grafo, aresta.destino, visitados, ordem); // Chamada recursiva para continuar a profundidade.
        }
    }
}

vector<string> buscaEmProfundidade(const ListaAdjacencia& grafo, const string& inicio) { // Inicializa e executa DFS a partir de um vertice.
    vector<string> ordem;
    set<string> visitados;

    if (grafo.find(inicio) != grafo.end()) {
        dfsRecursiva(grafo, inicio, visitados, ordem);
    }

    return ordem;
}

struct ResultadoDijkstra {
    int custo;
    vector<pair<string, string>> caminho;
};

// Calcula o menor caminho em grafo com pesos nao negativos e reconstrui as arestas usadas.
ResultadoDijkstra dijkstra(const ListaAdjacencia& grafo, const string& origem, const string& destino) { // Executa Dijkstra e reconstrui o caminho minimo.
    const int infinito = numeric_limits<int>::max(); // Valor sentinela para vertices ainda sem distancia.
    map<string, int> distancia; // Distancia minima conhecida desde a origem.
    map<string, string> anterior; // Predecessor de cada vertice no caminho minimo.

    for (const auto& item : grafo) { // Inicializa todos os vertices com distancia infinita.
        distancia[item.first] = infinito;
    }

    if (grafo.find(origem) == grafo.end() || grafo.find(destino) == grafo.end()) { // Falha rapida se origem/destino nao existirem.
        return {infinito, vector<pair<string, string>>()};
    }

    using Estado = pair<int, string>;
    priority_queue<Estado, vector<Estado>, greater<Estado>> fila; // Min-heap de (distancia, vertice).

    distancia[origem] = 0; // Distancia da origem para ela mesma eh zero.
    fila.push({0, origem}); // Inicia processamento pela origem.

    while (!fila.empty()) { // Processa vertices por menor distancia conhecida.
        int custoAtual = fila.top().first; // Menor custo extraido da fila.
        string atual = fila.top().second; // Vertice correspondente ao menor custo.
        fila.pop(); // Remove o estado da fila.

        if (custoAtual != distancia[atual]) { // Ignora estado obsoleto.
            continue;
        }

        if (atual == destino) { // Pode encerrar cedo ao atingir o destino.
            break;
        }

        for (const Aresta& aresta : grafo.at(atual)) { // Relaxa todas as arestas de saida do vertice atual.
            if (distancia[atual] == infinito) {
                continue;
            }

            int novaDistancia = distancia[atual] + aresta.peso; // Custo de chegar ao vizinho via vertice atual.
            bool melhoraCusto = novaDistancia < distancia[aresta.destino]; // Verdadeiro quando encontrou custo menor.
            bool empataComCaminhoLexicografico = // Desempate quando custo empata.
                novaDistancia == distancia[aresta.destino] &&
                (anterior.find(aresta.destino) == anterior.end() || atual < anterior[aresta.destino]);

            if (melhoraCusto || empataComCaminhoLexicografico) { // Atualiza melhor caminho conhecido para o vizinho.
                distancia[aresta.destino] = novaDistancia; // Grava nova melhor distancia.
                anterior[aresta.destino] = atual; // Guarda predecessor para reconstruir o caminho.
                fila.push({novaDistancia, aresta.destino}); // Reinsere na fila com o custo atualizado.
            }
        }
    }

    if (distancia[destino] == infinito) { // Sem caminho entre origem e destino.
        return {infinito, vector<pair<string, string>>()};
    }

    vector<string> verticesCaminho; // Sequencia de vertices do caminho minimo.
    for (string atual = destino; atual != origem; atual = anterior[atual]) { // Reconstrucao retrocedendo pelos predecessores.
        verticesCaminho.push_back(atual);
    }
    verticesCaminho.push_back(origem);
    reverse(verticesCaminho.begin(), verticesCaminho.end()); // Inverte para ordem origem -> destino.

    vector<pair<string, string>> arestasCaminho; // Converte vertices consecutivos em arestas.
    for (size_t i = 0; i + 1 < verticesCaminho.size(); ++i) { // Gera pares (u, v) do caminho.
        arestasCaminho.push_back({verticesCaminho[i], verticesCaminho[i + 1]});
    }

    return {distancia[destino], arestasCaminho};
}

void imprimirSequencia(const vector<string>& sequencia) { // Imprime uma sequencia de vertices separados por '->'.
    for (size_t i = 0; i < sequencia.size(); ++i) {
        cout << sequencia[i];
        if (i + 1 < sequencia.size()) {
            cout << " -> ";
        }
    }
    cout << '\n';
}

void imprimirCaminhoDijkstra(const ResultadoDijkstra& resultado) { // Mostra custo e arestas do caminho minimo encontrado.
    if (resultado.custo == numeric_limits<int>::max()) {
        cout << "Nao existe caminho entre os vertices informados.\n";
        return;
    }

    cout << "Custo total: " << resultado.custo << '\n';
    cout << "Arestas do caminho minimo: ";

    for (size_t i = 0; i < resultado.caminho.size(); ++i) {
        cout << "(" << resultado.caminho[i].first << ", " << resultado.caminho[i].second << ")";
        if (i + 1 < resultado.caminho.size()) {
            cout << " -> ";
        }
    }

    cout << '\n';
}

int main() { // Ponto de entrada: carrega grafos, executa algoritmos e imprime resultados.
    try {
        ListaAdjacencia g1 = lerGrafoNaoPonderado("g1.txt", false); // G1 nao dirigido e sem peso.
        ListaAdjacencia g2 = lerGrafoNaoPonderado("g2.txt", true); // G2 dirigido e sem peso.
        ListaAdjacencia g3 = lerGrafoPonderado("g3.txt", false); // G3 nao dirigido e ponderado.

        cout << "LISTA DE ADJACENCIA - G1 (nao dirigido)\n";
        imprimirListaAdjacencia(g1, false);
        cout << '\n';

        cout << "LISTA DE ADJACENCIA - G2 (dirigido)\n";
        imprimirListaAdjacencia(g2, false);
        cout << '\n';

        cout << "LISTA DE ADJACENCIA - G3 (nao dirigido e ponderado)\n";
        imprimirListaAdjacencia(g3, true);
        cout << '\n';

        cout << "BUSCA EM LARGURA EM G1 A PARTIR DO VERTICE b\n";
        vector<string> ordemBfs = buscaEmLargura(g1, "b"); // BFS em G1 a partir de b.
        imprimirSequencia(ordemBfs);
        cout << '\n';

        cout << "BUSCA EM PROFUNDIDADE EM G2 A PARTIR DO VERTICE a\n";
        vector<string> ordemDfs = buscaEmProfundidade(g2, "a"); // DFS em G2 a partir de a.
        imprimirSequencia(ordemDfs);
        cout << '\n';

        cout << "CAMINHO MINIMO EM G3 ENTRE x E t (DIJKSTRA)\n";
        ResultadoDijkstra resultado = dijkstra(g3, "x", "t"); // Menor caminho em G3 entre x e t.
        imprimirCaminhoDijkstra(resultado);
    } catch (const exception& erro) { // Captura erros de leitura/conversao e encerra com codigo de falha.
        cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }

    return 0;
}
