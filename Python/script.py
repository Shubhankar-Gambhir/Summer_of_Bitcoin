import cvxpy
import numpy as np
from collections import defaultdict
import sys 

# Class to represent a graph
class Graph:
    def __init__(self):
        self.graph = defaultdict(list)  # dictionary containing adjacency List
 
    def insert(self, u, v):            # function to add an edge to graph
        self.graph[u].append(v)
 
    def topologicalSortUtil(self, v, visited, stack): # A recursive function used by topologicalSort
 
        visited[v] = True # Mark the current node as visited.
 
        for i in self.graph[v]: # Recur for all the vertices adjacent to this vertex
            if visited[i] == False:
                self.topologicalSortUtil(i, visited, stack)
 
        stack.append(v) # Push current vertex to stack which stores result
 
    def topologicalSort(self):
        visited = self.graph.copy()  # Mark all the vertices as not visited
        for key in visited:
            visited[key] = False
        stack = []
        
        for i in visited:         # Sort starting from all vertices one by one
            if visited[i] == False:
                self.topologicalSortUtil(i, visited, stack) # Call the recursive helper function to store Topological
        original_stdout = sys.stdout
        # Print contents of the stack
        with open('block.txt', 'w') as f:
            sys.stdout = f # Change the standard output to the file we created.
            for t in stack[::-1]:
                print(t)
            sys.stdout = original_stdout 

def parse_mempool_csv():                                                # parses csv and updates all the lists and dictionary
    with open('../mempool.csv') as f:
        i = 0
        for line in f.readlines():
            parameters = line.strip().split(',')
            if(i):
                indices[parameters[0]] = i-1;
                Txn.append(parameters[0])
                fees.append(int(parameters[1]))
                weight.append(int(parameters[2]))
                parents.append(parameters[3].strip().split(';'))
            i += 1;

Txn = [];
indices = {};
weight = [];
fees  = [];
parents = [];

parse_mempool_csv()

B_SIZE = 4000000
W = np.array(weight)
F = np.array(fees)
selection = cvxpy.Variable(len(weight),boolean=True)          # set selection vector,For each transaction i its selection coefficient x_i = 0/1
c = cvxpy.sum(cvxpy.multiply(W,selection)) <= B_SIZE          # weight constraint
obj = cvxpy.sum(cvxpy.multiply(F,selection))                  # objective function
constraints = []
constraints.append(c)

validC = False

for i in range(len(weight)):
    C = np.zeros(len(weight))        
    for txid in parents[i]:
        if(txid != ''):
            C[i] += 1                                                   # if j,k,l are parent of i, then 3 * x_i <= x_j + x_k + x_l
            C[indices[txid]] -= 1                                       # or 3 * x_i - x_j - x_k - x_l <= 0;
            validC = True;                                              
    if (validC):
        c = cvxpy.sum(cvxpy.multiply(C,selection)) <= 0;
        constraints.append(c);                                          # add constraint ;
        validC = False
        

problem = cvxpy.Problem(cvxpy.Maximize(obj),constraints)
problem.solve(solver=cvxpy.GLPK_MI)                                     # set solver

g = Graph()

for i in range(len(weight)):
    if(selection.value[i]):                                             # add all selected nodes to graph with relavent edges
        g.insert(Txn[i],Txn[i])                                         # add an edge to self for including in graph, it wont affect topological sort
        for parent in parents[i]:
            if(parent != ''):
                g.insert(parent,Txn[i])
        
g.topologicalSort()             # so that parent gets printed before child
