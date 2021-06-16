#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<stack>
#include<sstream>
#include "ortools/linear_solver/linear_solver.h"

using namespace std;

#ifndef ONLINE_JUDGE
#else
#define debug(x...) 73
#endif

vector<int> fees;
vector<int> weight;
vector<string> txn;
vector<vector<string>> p_txn;
map<string,int> t_index;
vector<vector<int>> constraints;
vector<int> bounds;
vector<int> selection;
int n_txn = -1;
int n_constraints = 1;

// For topological sorting so that we can print elements such that parent is always printed before 
class Graph {
    map<string,vector<string>> adj;
    void topologicalSortUtil(string v, map<string,bool>& visited, stack<string>& Stack){
        // Mark the current node as visited.
        visited[v] = true;
    
        // Recur for all the vertices adjacent to this vertex
        for (auto i : adj[v])
            if (!visited[i])
                topologicalSortUtil(i, visited, Stack);
    
        // Push current vertex to stack which stores result
        Stack.push(v);
    }
  
    public:
    
        // function to add an edge to graph
        void addEdge(string v, string w){
            adj[v].push_back(w); // Add w to v’s list.
        }
    
        // prints a Topological Sort of the complete graph
        void topologicalSort(){
            stack<string> Stack;
        
            // Mark all the vertices as not visited
            map<string,bool> visited ;

            for (auto i : adj)
                visited[i.first] = false;
        
            // Call the recursive helper function to store Topological
            // Sort starting from all vertices one by one
            for (auto i : adj)
                if (visited[i.first] == false)
                    topologicalSortUtil(i.first, visited, Stack);
        
            // Print contents of stack
            while (Stack.empty() == false) {
                cout << Stack.top() << "\n";
                Stack.pop();
            }
        }
};

Graph g;

namespace operations_research {
    // function to solve Integer Linear Programing Problem
    void SolveILP(){

        // define solver type

        std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("scip"));
        if (!solver) {
            LOG(WARNING) << "SCIP solver unavailable.";
            return;
        }
        // declare variables for each transaction
        vector<const MPVariable*> x(txn.size());

        // variable lie in 0-1 (boolean values)
        for (int j = 0; j < n_txn; ++j)  x[j] = solver->MakeIntVar(0.0, 1.0, "");

        LOG(INFO) << "Number of variables = " << solver->NumVariables();

        // set constraint coefficients and bounds
        for (int i = 0; i < n_constraints; ++i) {
            MPConstraint* constraint = solver->MakeRowConstraint(0, bounds[i], "");
            for (int j = 0; j < n_txn ; ++j) {
                constraint->SetCoefficient(x[j], constraints[i][j]);
            }
        }
        LOG(INFO) << "Number of constraints = " << solver->NumConstraints();

        // Create the objective function.
        MPObjective* const objective = solver->MutableObjective();
        for (int j = 0; j < n_txn; ++j) objective->SetCoefficient(x[j], fees[j]);
        objective->SetMaximization();

        const MPSolver::ResultStatus result_status = solver->Solve();

        if (result_status != MPSolver::OPTIMAL) LOG(FATAL) << "The problem does not have an optimal solution.";

        LOG(INFO) << "Solution:";
        LOG(INFO) << "Optimal objective value = " << int(objective->Value());

        for (int j = 0; j < n_txn; ++j) {
            // if transaction is slected add it in graph
            if(x[j]->solution_value()){
                g.addEdge(txn[j],txn[j]); // to include edge so we can add it graph
                for(auto p: p_txn[j]){
                    g.addEdge(p,txn[j]); // the corresponding edge in graph
                }
            }

        }
    }
}


void get_constraints(){
    constraints.push_back(weight); // set weight constraints
    bounds.push_back(4000000);
    for(int i = 0; i < n_txn;i++){
        vector<int> C(n_txn,0); 

        if(p_txn[i].size()){                                            // For each transaction i let its selection coefficient be denoted by x_i, x_i = 0/1
            for (auto txid : p_txn[i]){
                C[i]++;                                                 // if j,k,l are parent of i, then 3 * x_i <= x_j + x_k + x_l 
                C[t_index[txid]]--;                                     // or 3 * x_i - x_j - x_k - x_l <= 0;
            }
            constraints.push_back(C);                                   // add constraint and bounds;
            bounds.push_back(0);
            n_constraints++;
        }
    }
        
}

void parse_csv(){
    string line,word;
    while(getline(cin,line)){                                       // read input line by line
        vector<string> row;
        vector<string> parent;  
        if(n_txn!=-1){                                              // ignore first line as it has headings
            stringstream s(line);       
            while (getline(s, word, ',')) row.push_back(word);

            if(row.size()==4){                                      // if row has parents add them in p_txns else add an empty vector
                stringstream s1(row[3]);
                while(getline(s1,word,';')) parent.push_back(word);
            }   

            txn.push_back(row[0]);                                  // add data in corresponding arrays
            t_index[row[0]] = n_txn;
            fees.push_back(stoi(row[1]));
            weight.push_back(stoi(row[2]));      
            p_txn.push_back(parent);        

        }
        n_txn++ ;
    }
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);

#ifndef ONLINE_JUDGE
    freopen("../../mempool.csv", "r", stdin); // add path of files relative to directory "or tools" here
    freopen("../block.txt", "w", stdout);   
#endif
    
    parse_csv();                                        // parse CSV
    get_constraints();                                  // get required constraints
    operations_research::SolveILP();                    // solve them
    g.topologicalSort();
    cerr << '\n' << "Time : " << 1000 * ((double)clock()) / (double)CLOCKS_PER_SEC << "ms\n";
    return 0;
}