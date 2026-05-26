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

string removerEspacosDasPontas(const string& texto) {
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
vector<string> separarPorPontoEVirgula(const string& linha) {
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

void adicionarVerticeSeNecessario(ListaAdjacencia& grafo, const string& vertice) {
    if (grafo.find(vertice) == grafo.end()) {
        grafo[vertice] = vector<Aresta>();
    }
}

void adicionarAresta(ListaAdjacencia& grafo, const string& origem, const string& destino, int peso) {
    adicionarVerticeSeNecessario(grafo, origem);
    adicionarVerticeSeNecessario(grafo, destino);
    grafo[origem].push_back({destino, peso});
}

void ordenarListas(ListaAdjacencia& grafo) {
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
ListaAdjacencia lerGrafoNaoPonderado(const string& nomeArquivo, bool dirigido) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo " + nomeArquivo);
    }

    ListaAdjacencia grafo;
    string linha;

    while (getline(arquivo, linha)) {
        vector<string> campos = separarPorPontoEVirgula(linha);
        if (campos.size() < 2) {
            continue;
        }

        adicionarAresta(grafo, campos[0], campos[1], 1);
        if (!dirigido) {
            adicionarAresta(grafo, campos[1], campos[0], 1);
        }
    }

    ordenarListas(grafo);
    return grafo;
}

// Le grafo ponderado usando o terceiro campo da linha como custo da aresta.
ListaAdjacencia lerGrafoPonderado(const string& nomeArquivo, bool dirigido) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo " + nomeArquivo);
    }

    ListaAdjacencia grafo;
    string linha;

    while (getline(arquivo, linha)) {
        vector<string> campos = separarPorPontoEVirgula(linha);
        if (campos.size() < 3) {
            continue;
        }

        int peso = stoi(campos[2]);
        adicionarAresta(grafo, campos[0], campos[1], peso);
        if (!dirigido) {
            adicionarAresta(grafo, campos[1], campos[0], peso);
        }
    }

    ordenarListas(grafo);
    return grafo;
}

void imprimirListaAdjacencia(const ListaAdjacencia& grafo, bool ponderado) {
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
vector<string> buscaEmLargura(const ListaAdjacencia& grafo, const string& inicio) {
    vector<string> ordem;
    set<string> visitados;

    if (grafo.find(inicio) == grafo.end()) {
        return ordem;
    }

    visitados.insert(inicio);
    vector<string> camadaAtual;
    camadaAtual.push_back(inicio);

    while (!camadaAtual.empty()) {
        sort(camadaAtual.begin(), camadaAtual.end());
        set<string> proximaCamada;

        for (const string& atual : camadaAtual) {
            ordem.push_back(atual);

            for (const Aresta& aresta : grafo.at(atual)) {
                if (visitados.find(aresta.destino) == visitados.end()) {
                    visitados.insert(aresta.destino);
                    proximaCamada.insert(aresta.destino);
                }
            }
        }

        camadaAtual.assign(proximaCamada.begin(), proximaCamada.end());
    }

    return ordem;
}

// DFS recursiva seguindo a lista de adjacencia previamente ordenada.
void dfsRecursiva(const ListaAdjacencia& grafo, const string& atual, set<string>& visitados, vector<string>& ordem) {
    visitados.insert(atual);
    ordem.push_back(atual);

    auto it = grafo.find(atual);
    if (it == grafo.end()) {
        return;
    }

    for (const Aresta& aresta : it->second) {
        if (visitados.find(aresta.destino) == visitados.end()) {
            dfsRecursiva(grafo, aresta.destino, visitados, ordem);
        }
    }
}

vector<string> buscaEmProfundidade(const ListaAdjacencia& grafo, const string& inicio) {
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
ResultadoDijkstra dijkstra(const ListaAdjacencia& grafo, const string& origem, const string& destino) {
    const int infinito = numeric_limits<int>::max();
    map<string, int> distancia;
    map<string, string> anterior;

    for (const auto& item : grafo) {
        distancia[item.first] = infinito;
    }

    if (grafo.find(origem) == grafo.end() || grafo.find(destino) == grafo.end()) {
        return {infinito, vector<pair<string, string>>()};
    }

    using Estado = pair<int, string>;
    priority_queue<Estado, vector<Estado>, greater<Estado>> fila;

    distancia[origem] = 0;
    fila.push({0, origem});

    while (!fila.empty()) {
        int custoAtual = fila.top().first;
        string atual = fila.top().second;
        fila.pop();

        if (custoAtual != distancia[atual]) {
            continue;
        }

        if (atual == destino) {
            break;
        }

        for (const Aresta& aresta : grafo.at(atual)) {
            if (distancia[atual] == infinito) {
                continue;
            }

            int novaDistancia = distancia[atual] + aresta.peso;
            bool melhoraCusto = novaDistancia < distancia[aresta.destino];
            bool empataComCaminhoLexicografico =
                novaDistancia == distancia[aresta.destino] &&
                (anterior.find(aresta.destino) == anterior.end() || atual < anterior[aresta.destino]);

            if (melhoraCusto || empataComCaminhoLexicografico) {
                distancia[aresta.destino] = novaDistancia;
                anterior[aresta.destino] = atual;
                fila.push({novaDistancia, aresta.destino});
            }
        }
    }

    if (distancia[destino] == infinito) {
        return {infinito, vector<pair<string, string>>()};
    }

    vector<string> verticesCaminho;
    for (string atual = destino; atual != origem; atual = anterior[atual]) {
        verticesCaminho.push_back(atual);
    }
    verticesCaminho.push_back(origem);
    reverse(verticesCaminho.begin(), verticesCaminho.end());

    vector<pair<string, string>> arestasCaminho;
    for (size_t i = 0; i + 1 < verticesCaminho.size(); ++i) {
        arestasCaminho.push_back({verticesCaminho[i], verticesCaminho[i + 1]});
    }

    return {distancia[destino], arestasCaminho};
}

void imprimirSequencia(const vector<string>& sequencia) {
    for (size_t i = 0; i < sequencia.size(); ++i) {
        cout << sequencia[i];
        if (i + 1 < sequencia.size()) {
            cout << " -> ";
        }
    }
    cout << '\n';
}

void imprimirCaminhoDijkstra(const ResultadoDijkstra& resultado) {
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

int main() {
    try {
        ListaAdjacencia g1 = lerGrafoNaoPonderado("g1.txt", false);
        ListaAdjacencia g2 = lerGrafoNaoPonderado("g2.txt", true);
        ListaAdjacencia g3 = lerGrafoPonderado("g3.txt", true);

        cout << "LISTA DE ADJACENCIA - G1 (nao dirigido)\n";
        imprimirListaAdjacencia(g1, false);
        cout << '\n';

        cout << "LISTA DE ADJACENCIA - G2 (dirigido)\n";
        imprimirListaAdjacencia(g2, false);
        cout << '\n';

        cout << "LISTA DE ADJACENCIA - G3 (dirigido e ponderado)\n";
        imprimirListaAdjacencia(g3, true);
        cout << '\n';

        cout << "BUSCA EM LARGURA EM G1 A PARTIR DO VERTICE b\n";
        vector<string> ordemBfs = buscaEmLargura(g1, "b");
        imprimirSequencia(ordemBfs);
        cout << '\n';

        cout << "BUSCA EM PROFUNDIDADE EM G2 A PARTIR DO VERTICE a\n";
        vector<string> ordemDfs = buscaEmProfundidade(g2, "a");
        imprimirSequencia(ordemDfs);
        cout << '\n';

        cout << "CAMINHO MINIMO EM G3 ENTRE x E t (DIJKSTRA)\n";
        ResultadoDijkstra resultado = dijkstra(g3, "x", "t");
        imprimirCaminhoDijkstra(resultado);
    } catch (const exception& erro) {
        cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }

    return 0;
}
