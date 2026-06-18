#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_NAME_LEN 32
#define MAX_LINE_NAME 20

/* 邻接表边结点 */
typedef struct EdgeNode {
    int adjVex;
    int weight;
    struct EdgeNode *next;
} EdgeNode;

/* 顶点结点（站点） */
typedef struct VertexNode {
    char name[MAX_NAME_LEN];
    EdgeNode *firstEdge;
    int *lineIds;
    int lineCount;
} VertexNode;

/* 图结构 */
typedef struct {
    VertexNode *vertices;
    int vertexNum;
    int vertexCapacity;
    int edgeNum;
    int isDirected;
} Graph;

/* 辅助队列（BFS） */
typedef struct Queue {
    int *data;
    int front, rear, size, capacity;
} Queue;

/* 函数声明 */
Graph* createGraph(int initCapacity, int isDirected);
void resizeGraph(Graph *g);
int addVertex(Graph *g, const char *name);
int findVertexIndex(Graph *g, const char *name);
void addEdge(Graph *g, int u, int v, int weight);
void addLineToStation(Graph *g, int stationIdx, int lineId);
void readMetroFile(const char *filename, Graph *g);
void printAdjList(Graph *g);

/* ========== TODO ========== */
void DFSRecursive(Graph *g, int v, int *visited);
void DFSTraversal(Graph *g, int start);
void BFSTraversal(Graph *g, int start);
void connectivityAnalysis(Graph *g);
void dijkstra(Graph *g, int start, int *dist, int *prev);
void printPath(Graph *g, int *prev, int start, int end);
void shortestPathByTime(Graph *g, int start, int end);
void shortestPathByTransfer(Graph *g, int start, int end);
void freeGraph(Graph *g);

/* 队列 */
Queue* createQueue(int capacity);
void enqueue(Queue *q, int val);
int dequeue(Queue *q);
int isEmpty(Queue *q);
void freeQueue(Queue *q);

/* ======================= 主函数 ======================= */
int main() {
    Graph *g = createGraph(100, 0);
    readMetroFile("metro.txt", g);

    int choice, start, end;
    char startName[MAX_NAME_LEN], endName[MAX_NAME_LEN];

    do {
        printf("\n====== 地铁查询系统 ======\n");
        printf("1. 输出邻接表和换乘站\n");
        printf("2. DFS 遍历（从指定站点）\n");
        printf("3. BFS 遍历（从指定站点）\n");
        printf("4. 连通分量分析\n");
        printf("5. 最短路径（最少时间）\n");
        printf("6. 最短路径（最少换乘）\n");
        printf("0. 退出\n");
        printf("请输入选择：");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printAdjList(g);
                break;
            case 2:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) printf("站点不存在！\n");
                else {
                    printf("DFS 遍历序列：");
                    DFSTraversal(g, start);
                }
                break;
            case 3:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) printf("站点不存在！\n");
                else {
                    printf("BFS 遍历序列：");
                    BFSTraversal(g, start);
                }
                break;
            case 4:
                connectivityAnalysis(g);
                break;
            case 5:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1 || end == -1)
                    printf("站点不存在！\n");
                else
                    shortestPathByTime(g, start, end);
                break;
            case 6:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1 || end == -1)
                    printf("站点不存在！\n");
                else
                    shortestPathByTransfer(g, start, end);
                break;
            case 0:
                printf("退出程序。\n");
                break;
            default:
                printf("无效选择！\n");
        }
    } while (choice != 0);

    freeGraph(g);
    return 0;
}

/* ======================= 已给实现 ======================= */
Graph* createGraph(int initCapacity, int isDirected) {
    Graph *g = malloc(sizeof(Graph));
    g->vertexCapacity = initCapacity;
    g->vertexNum = 0;
    g->edgeNum = 0;
    g->isDirected = isDirected;
    g->vertices = malloc(sizeof(VertexNode) * initCapacity);
    for (int i = 0; i < initCapacity; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    return g;
}

void resizeGraph(Graph *g) {
    int newCap = g->vertexCapacity * 2;
    g->vertices = realloc(g->vertices, sizeof(VertexNode) * newCap);
    for (int i = g->vertexCapacity; i < newCap; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    g->vertexCapacity = newCap;
}

int addVertex(Graph *g, const char *name) {
    int idx = findVertexIndex(g, name);
    if (idx != -1) return idx;

    if (g->vertexNum >= g->vertexCapacity)
        resizeGraph(g);

    strcpy(g->vertices[g->vertexNum].name, name);
    g->vertices[g->vertexNum].firstEdge = NULL;
    g->vertices[g->vertexNum].lineIds = NULL;
    g->vertices[g->vertexNum].lineCount = 0;
    return g->vertexNum++;
}

int findVertexIndex(Graph *g, const char *name) {
    for (int i = 0; i < g->vertexNum; i++)
        if (strcmp(g->vertices[i].name, name) == 0)
            return i;
    return -1;
}

void addEdge(Graph *g, int u, int v, int weight) {
    EdgeNode *e = malloc(sizeof(EdgeNode));
    e->adjVex = v;
    e->weight = weight;
    e->next = g->vertices[u].firstEdge;
    g->vertices[u].firstEdge = e;

    if (!g->isDirected) {
        e = malloc(sizeof(EdgeNode));
        e->adjVex = u;
        e->weight = weight;
        e->next = g->vertices[v].firstEdge;
        g->vertices[v].firstEdge = e;
    }
    g->edgeNum++;
}

void addLineToStation(Graph *g, int stationIdx, int lineId) {
    VertexNode *v = &g->vertices[stationIdx];
    for (int i = 0; i < v->lineCount; i++)
        if (v->lineIds[i] == lineId) return;

    v->lineCount++;
    v->lineIds = realloc(v->lineIds, v->lineCount * sizeof(int));
    v->lineIds[v->lineCount - 1] = lineId;
}

void readMetroFile(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("无法打开文件 %s\n", filename);
        exit(1);
    }

    char line[256];
    int routeCount;
    while (fgets(line, sizeof(line), fp))
        if (line[0] != '#' && line[0] != '\n') {
            sscanf(line, "%d", &routeCount);
            break;
        }
    fgets(line, sizeof(line), fp);

    for (int rid = 0; rid < routeCount; rid++) {
        if (!fgets(line, sizeof(line), fp)) break;
        if (line[0] == '#' || line[0] == '\n') { rid--; continue; }

        char lineName[MAX_LINE_NAME];
        int stationCount;
        char *token = strtok(line, " \t\n");
        strcpy(lineName, token);
        token = strtok(NULL, " \t\n");
        stationCount = atoi(token);

        int prev = -1, time = 1;
        for (int i = 0; i < stationCount; i++) {
            token = strtok(NULL, " \t\n");
            if (!token) break;

            int isTime = 1;
            for (char *p = token; *p; p++)
                if (!isdigit(*p)) { isTime = 0; break; }

            if (isTime && i > 0) {
                time = atoi(token);
                continue;
            }

            int idx = addVertex(g, token);
            addLineToStation(g, idx, rid);

            if (prev != -1)
                addEdge(g, prev, idx, time);
            prev = idx;
            time = 1;
        }
    }
    fclose(fp);
    printf("成功读取地铁数据：%d 个站点，%d 条边。\n", g->vertexNum, g->edgeNum);
}

void printAdjList(Graph *g) {
    printf("\n===== 邻接表 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        printf("%s：", g->vertices[i].name);
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            printf("-> %s(%dmin) ", g->vertices[e->adjVex].name, e->weight);
            e = e->next;
        }
        printf("\n");
    }
}

/* ======================= TODO 实现 ======================= */

void DFSRecursive(Graph *g, int v, int *visited) {
    visited[v] = 1;
    printf("%s ", g->vertices[v].name);

    EdgeNode *e = g->vertices[v].firstEdge;
    while (e) {
        if (!visited[e->adjVex])
            DFSRecursive(g, e->adjVex, visited);
        e = e->next;
    }
}

void DFSTraversal(Graph *g, int start) {
    int *visited = calloc(g->vertexNum, sizeof(int));
    DFSRecursive(g, start, visited);
    printf("\n");
    free(visited);
}

void BFSTraversal(Graph *g, int start) {
    int *visited = calloc(g->vertexNum, sizeof(int));
    Queue *q = createQueue(g->vertexNum);

    visited[start] = 1;
    enqueue(q, start);

    while (!isEmpty(q)) {
        int u = dequeue(q);
        printf("%s ", g->vertices[u].name);

        EdgeNode *e = g->vertices[u].firstEdge;
        while (e) {
            if (!visited[e->adjVex]) {
                visited[e->adjVex] = 1;
                enqueue(q, e->adjVex);
            }
            e = e->next;
        }
    }
    printf("\n");
    freeQueue(q);
    free(visited);
}

void connectivityAnalysis(Graph *g) {
    int *visited = calloc(g->vertexNum, sizeof(int));
    int cnt = 0;

    for (int i = 0; i < g->vertexNum; i++) {
        if (!visited[i]) {
            cnt++;
            printf("连通分量 %d：", cnt);
            Queue *q = createQueue(g->vertexNum);
            visited[i] = 1;
            enqueue(q, i);

            while (!isEmpty(q)) {
                int u = dequeue(q);
                printf("%s ", g->vertices[u].name);
                EdgeNode *e = g->vertices[u].firstEdge;
                while (e) {
                    if (!visited[e->adjVex]) {
                        visited[e->adjVex] = 1;
                        enqueue(q, e->adjVex);
                    }
                    e = e->next;
                }
            }
            printf("\n");
            freeQueue(q);
        }
    }
    printf("共 %d 个连通分量\n", cnt);
    free(visited);
}

void dijkstra(Graph *g, int start, int *dist, int *prev) {
    int n = g->vertexNum;
    int *vis = calloc(n, sizeof(int));

    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int i = 0; i < n; i++) {
        int u = -1, min = INT_MAX;
        for (int j = 0; j < n; j++)
            if (!vis[j] && dist[j] < min) {
                min = dist[j];
                u = j;
            }
        if (u == -1) break;
        vis[u] = 1;

        EdgeNode *e = g->vertices[u].firstEdge;
        while (e) {
            int v = e->adjVex;
            if (!vis[v] && dist[u] + e->weight < dist[v]) {
                dist[v] = dist[u] + e->weight;
                prev[v] = u;
            }
            e = e->next;
        }
    }
    free(vis);
}

void printPath(Graph *g, int *prev, int start, int end) {
    if (end == start) {
        printf("%s", g->vertices[start].name);
        return;
    }
    if (prev[end] == -1) {
        printf("无法到达");
        return;
    }
    printPath(g, prev, start, prev[end]);
    printf("->%s", g->vertices[end].name);
}

void shortestPathByTime(Graph *g, int start, int end) {
    int *dist = malloc(g->vertexNum * sizeof(int));
    int *prev = malloc(g->vertexNum * sizeof(int));

    dijkstra(g, start, dist, prev);

    if (dist[end] == INT_MAX)
        printf("无法到达终点\n");
    else {
        printf("最短时间路径（%d 分钟）：", dist[end]);
        printPath(g, prev, start, end);
        printf("\n");
    }
    free(dist);
    free(prev);
}

void shortestPathByTransfer(Graph *g, int start, int end) {
    /* 保存原权值 */
    int n = g->vertexNum;
    int edgeCnt = 0;
    EdgeNode **edges = NULL;
    int *weights = NULL;

    for (int i = 0; i < n; i++) {
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            edges = realloc(edges, (edgeCnt + 1) * sizeof(EdgeNode *));
            weights = realloc(weights, (edgeCnt + 1) * sizeof(int));
            edges[edgeCnt] = e;
            weights[edgeCnt] = e->weight;
            e->weight = 1;
            edgeCnt++;
            e = e->next;
        }
    }

    int *dist = malloc(n * sizeof(int));
    int *prev = malloc(n * sizeof(int));
    dijkstra(g, start, dist, prev);

    if (dist[end] == INT_MAX)
        printf("无法到达终点\n");
    else {
        printf("最少换乘路径（换乘 %d 次）：", dist[end] - 1);
        printPath(g, prev, start, end);
        printf("\n");
    }

    /* 恢复权值 */
    for (int i = 0; i < edgeCnt; i++)
        edges[i]->weight = weights[i];

    free(dist);
    free(prev);
    free(edges);
    free(weights);
}

void freeGraph(Graph *g) {
    for (int i = 0; i < g->vertexNum; i++) {
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            EdgeNode *tmp = e;
            e = e->next;
            free(tmp);
        }
        free(g->vertices[i].lineIds);
    }
    free(g->vertices);
    free(g);
}

/* ======================= 队列 ======================= */
Queue* createQueue(int capacity) {
    Queue *q = malloc(sizeof(Queue));
    q->data = malloc(sizeof(int) * capacity);
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int val) {
    if (q->size == q->capacity) return;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    int v = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return v;
}

int isEmpty(Queue *q) {
    return q->size == 0;
}

void freeQueue(Queue *q) {
    free(q->data);
    free(q);
}
