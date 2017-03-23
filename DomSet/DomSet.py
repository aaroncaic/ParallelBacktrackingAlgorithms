import networkx as nx
import time

sol = []

def put(G, n, i, num_zeros):
    new_num_zeros = num_zeros

    if G.node[n]["dominated"] == 0:
        new_num_zeros = new_num_zeros - 1
    G.node[n]["dominated"] = i

    for m in G.neighbors(n):
        if G.node[m]["dominated"] > 0:
            continue
        if G.node[m]["dominated"] == 0:
            new_num_zeros = new_num_zeros - 1
        G.node[m]["dominated"] = -i

    return new_num_zeros

def dom_set_helper(G, k, i, num_zeros):
    if i >= k:
        return
    global sol

    for n in G.nodes():
        if sol:
            return
        if G.node[n]["marked"] == 1 or G.node[n]["dominated"] > 0:
            print("skip")
            continue
        if i == 0:
            G.node[n]["marked"] = 1

        H = G.copy()
        new_num_zeros = put(H, n, i+1, num_zeros)

        if new_num_zeros == 0 and i+1 == k:
            sol = H
            return

        dom_set_helper(H, k, i+1, new_num_zeros)


    return

def dom_set(G, k):
    H = G.copy()
    for n in H:
        H.node[n]["dominated"] = 0
        H.node[n]["marked"] = 0
    dom_set_helper(H, k, 0, H.number_of_nodes())
    dom_set = []
    for n in sol:
        if sol.node[n]["dominated"] > 0:
            dom_set.append(n)
    return dom_set

# Create chess board for min dom set of queens
def make_chess_board(n):
    G = nx.Graph()
    graph_size = n*n
    for i in range(0,graph_size):
        G.add_node(i)
    for i in range(0,graph_size):
        j = i + n
        while(j < graph_size):
            G.add_edge(i,j)
            j = j + n
        j = i - n
        while(j >= 0):
            G.add_edge(i,j)
            j = j - n
        j = i + 1
        row_end = ((int)(i / n)) * n + n
        while(j < row_end):
            G.add_edge(i,j)
            j = j + 1
        j = i - 1
        row_start = ((int)(i / n)) * n
        while(j >= row_start):
            G.add_edge(i,j)
            j = j - 1

        row_end = ((int)((i+n) / n)) * n + n
        j = i + n + 1
        while(j < row_end and j < graph_size):
            G.add_edge(i,j)
            row_end = ((int)((j+n) / n)) * n + n
            j = j + n + 1
        row_start = ((int)((i-n) / n)) * n
        j = i - n - 1
        while(j >= row_start and j >= 0):
            G.add_edge(i,j)
            row_start = ((int)((j-n) / n)) * n
            j = j - n - 1
        row_start = ((int)((i+n) / n)) * n
        j = i + n - 1
        while(j >= row_start and j < graph_size):
            G.add_edge(i,j)
            row_start = ((int)((j+n) / n)) * n
            j = j + n - 1
        row_end = ((int)((i-n) / n)) * n + n
        j = i - n + 1
        while(j < row_end and j >= 0):
            G.add_edge(i,j)
            row_end = ((int)((j-n) / n)) * n + n
            j = j - n + 1
    return G


# TEST CASES
#A = nx.Graph()
#A.add_nodes_from([1,2,3,4,5,6,7])
#A.add_edges_from([(1,2),(2,3),(3,4),(1,5),(5,6),(5,2),(6,3),(4,7)])
A = make_chess_board(6)

start_time = time.time()
dom = dom_set(A,3)
end_time = time.time()
print("--- %s seconds ---" % (end_time - start_time))
print(dom)

# Test out the built in dominating set package
#start_time = time.time()
#set_of_dom = nx.dominating_set(A)
#end_time = time.time()
#print("--- %s seconds ---\n" % (end_time - start_time))
#print(set_of_dom)
