import networkx as nx
import time
import math


n = -1              # Number of nodes
n_dominated = 0     # Number of nodes dominated
num_choice = []     # Possible number of times node can be dominated
num_dominated = []  # Number of time node has been dominated
dom = []            # Current set of dominating nodes
size = 0            # Size of dominating set
min_dom = []        # Minimum dominating set of nodes
min_size = -1       # Size of minimum dominating set
max_degree = 0      # Max degree of a node in graph


def min_dom_set_helper(G, level):
    global num_choice

    # Return if any node can no longer be dominated
    for i in range(len(num_choice)):
        if num_choice[i] <= 0:
            return

    global n
    global n_dominated
    global size
    global min_size
    global min_dom
    global dom

    # Found a dominating set
    if level == n or n_dominated == n:
        if size < min_size:
            min_dom = list(dom)
            min_size = size
        return

    global max_degree

    # Heuristic
    if(size + math.ceil(float(n-n_dominated)/(max_degree+1)) >= min_size):
        return

    # Set current node as not-part-of-dominating-set
    u = level
    num_choice[u] = num_choice[u] - 1
    for v in G.neighbors(u):
        num_choice[v] = num_choice[v] - 1

    # Call recursively
    min_dom_set_helper(G, level+1)

    # Restore datastructure
    for v in G.neighbors(u):
        num_choice[v] = num_choice[v] + 1
    num_choice[u] = num_choice[u] + 1

    global num_dominated

    # Add current node to dominating set
    dom.append(u)
    size = size + 1
    if(num_dominated[u] == 0):
        n_dominated = n_dominated + 1
    num_dominated[u] = num_dominated[u] + 1
    for v in G.neighbors(u):
        if(num_dominated[v] == 0):
            n_dominated = n_dominated + 1
        num_dominated[v] = num_dominated[v] + 1

    # Call recursively
    min_dom_set_helper(G, level+1)

    # Restore datastructure by removing node from dominating set
    for v in G.neighbors(u):
        num_dominated[v] = num_dominated[v] - 1
        if(num_dominated[v] == 0):
            n_dominated = n_dominated - 1
    num_dominated[u] = num_dominated[u] - 1
    if(num_dominated[u] == 0):
        n_dominated = n_dominated - 1
    size = size - 1
    dom.pop()

    return

def min_dom_set(G):
    H = G.copy()
    global n
    n = H.number_of_nodes()
    global n_dominated
    n_dominated = 0
    global num_choice
    num_choice = [None]*n
    global num_dominated
    num_dominated = [None]*n
    global min_size
    min_size = n
    global min_dom
    min_dom = [None]*n
    global max_degree

    for i in range(len(num_choice)):
        num_choice[i] = H.degree(i) + 1
        if H.degree(i) > max_degree:
            max_degree = H.degree(i)
    for i in range(len(num_dominated)):
        num_dominated[i] = 0
    for i in range(len(min_dom)):
        min_dom[i] = i

    min_dom_set_helper(H, 0)

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
G = make_chess_board(6)
#G = nx.octahedral_graph()
#G = nx.dodecahedral_graph()
#G = nx.cubical_graph()
#G = nx.random_regular_graph(2,50)
#G = nx.complete_graph(100)

# Peterson Graph
#G.add_nodes_from([0,1,2,3,4,5,6,7,8,9])
#G.add_path([0,1,6,7,3])
#G.add_edge(0,3)
#G.add_path([4,5,8,2,9])
#G.add_edge(9,4)
#G.add_edges_from([(0,2),(1,4),(3,5),(7,9),(6,8)])

start_time = time.time()
min_dom_set(G)
end_time = time.time()
print("%s seconds" % (end_time - start_time))
print(min_dom)
if nx.is_dominating_set(G, min_dom):
    print("Valid Solution")
