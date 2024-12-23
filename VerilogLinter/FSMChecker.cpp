#include "FSMChecker.h"

FSMChecker::FSMChecker(QObject *parent)
    : QObject{parent}
{}

void FSMChecker::findUnreachableStates(const FSM &fsm) {
    // Create a graph representation
    unordered_map<string, vector<string>> graph;
    for (const auto& transition : fsm.transitions) {
        graph[transition.from_state].push_back(transition.to_state);
    }

    // Perform BFS to find reachable states
    unordered_set<string> reachable_states;
    queue<string> q;

    // Start BFS from the initial state
    q.push(fsm.initial_state);
    reachable_states.insert(fsm.initial_state);

    while (!q.empty()) {
        string current = q.front();
        q.pop();

        for (const auto& neighbor : graph[current]) {
            if (reachable_states.find(neighbor) == reachable_states.end()) {
                reachable_states.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    // Find unreachable states
    unordered_set<string> unreachable_states;
    for (const auto& state : fsm.states) {
        if (reachable_states.find(state) == reachable_states.end()) {
            unreachable_states.insert(state);
        }
    }

    //cout << "\nUnreachable States:\n";
    for (const auto& state : unreachable_states) {
        //cout << state << endl;
        violations.push_back({"Unreachable FSM State", -1, "unreachable state: "+state});
    }
}
