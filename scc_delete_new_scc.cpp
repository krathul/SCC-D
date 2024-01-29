// will test out different ways to create the new scc for the vert not part of original scc

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "aux.cpp"

struct pair_hash {
    inline std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first*31+v.second;
    }
};

typedef unordered_set<pair<int, int>, pair_hash> del_set;

bool not_deleted(int src, int dst, del_set deleted_edges){
	pair temp = make_pair(src, dst);
	if(deleted_edges.find(temp)!=deleted_edges.end()){
		return false;
	}
	else{
		return true;
	}
}

void fw_new(int node, graph& g, int* fw_reach, int* reachable, del_set deleted_edges){ //TO:DO can be combined with fw_propagate in the other file, need to implement that
#if DEBUG
	printf("fw search %d\n",node);
#endif
	int temp_size = g.count_in_sccs[find_index(g.scc_map[node], g.rep_nodes, g.scc_count)];
	int* queue = new int[temp_size];
	int* nxt_queue = new int[temp_size]; // TO:DO figure out the sizes properly
	int q_size=0, nxt_q_size = 0;
	queue[q_size++] = node;
	while(q_size){
		for(int i=0; i<q_size; i++){ //doing separately to ensure we dont repeat vertices later
			int vert = queue[i];
			fw_reach[vert] = vert;
		}
		for(int i=0; i<q_size; i++){
			int vert = queue[i];
			int Odeg = out_degree(g, vert);
			int* Overts = out_vertices(g, vert);
			for(int j=0; j<Odeg; j++){
				int nVert = Overts[i];
				if(!reachable[nVert] and not_deleted(vert, nVert, deleted_edges) and g.scc_map[vert]==g.scc_map[nVert] and fw_reach[nVert]!=node){
					nxt_queue[nxt_q_size++] = nVert;
				}
			}
		}
		swap(queue, nxt_queue);
		q_size = nxt_q_size;
		nxt_q_size = 0;
	}
#if DEBUG
	printf("done fw search\n");
#endif
	delete queue;
	delete nxt_queue;
}

void find_scc(int node, int* to_change, int& num_changed, graph& g, int* reachable, del_set& deleted_edges, int* fw_reach){ //will return the list of verts that needs scc changed
#if DEBUG
	printf("Finding SCC of %d\n", node);
#endif
	int temp_size = g.count_in_sccs[find_index(g.scc_map[node], g.rep_nodes, g.scc_count)];
	int* queue = new int[temp_size];
	int* nxt_queue = new int[temp_size]; // TO:DO figure out the sizes properly
	int q_size=0, nxt_q_size=0;
	queue[q_size++] = node;
	while(q_size){
		for(int i=0; i<q_size; i++){
			int vert = queue[i];
#if DEBUG
			printf("Marking %d reachable as reachable from %d\n",vert,node);
#endif
			reachable[vert] = 1;
			to_change[num_changed++] = vert; //adding this vertex beacuse it needs scc change
			int Ideg = in_degree(g, vert);
			int* Iverts = in_vertices(g, vert);
			for(int j=0; j<Ideg; j++){
				int nVert = Iverts[i];
				if(not_deleted(nVert, vert, deleted_edges) and vert==fw_reach[nVert] and !reachable[nVert]){ // last confition might be redundant
					nxt_queue[nxt_q_size++] = nVert;
				}
			}
		}
		swap(queue, nxt_queue);
		q_size = nxt_q_size;
		nxt_q_size=0;
	}

	delete queue;
	delete nxt_queue;
}

void update_sccs(int vert, int* to_change, int size, graph& g){
#if DEBUG
	printf("updating scc for %d\n", vert);
#endif
	for(int i=0; i<size; i++){
		g.scc_map[to_change[i]] = vert;
	}
}

void make_scc(int vert, graph& g, int* fw_reach, int* reachable, del_set deleted_edges){
#if DEBUG
	printf("Starting to make scc of vertex %d\n", vert);
#endif
	fw_new(vert, g, fw_reach, reachable, deleted_edges);
	int num_changed=0;
	int* to_change = new int[g.count_in_sccs[find_index(g.scc_map[vert], g.rep_nodes, g.scc_count)]]; //TO:DO will decide size
	find_scc(vert, to_change, num_changed, g, reachable, deleted_edges, fw_reach);
	update_sccs(vert, to_change, num_changed, g);
#if DEBUG
	printf("done making scc\n");
#endif
	delete [] to_change;
}

void create_new_sccs(int* unreachable_verts, int* reachable, int& num_vrts, graph& g, int* fw_reach, del_set deleted_edges){ // currently we go vertex by vertex and form a new scc for that particular vertex
#if DEBUG
	printf("starting to create new sccs\n");
#endif
	for(int i=0; i<num_vrts; i++){ // this part not yet to parallelise due to potential conflict in forward update
		int vert = unreachable_verts[i];
		if(!reachable[vert]){
			make_scc(vert, g, fw_reach, reachable, deleted_edges);
		}
	}
#if DEBUG
	printf("Finished creating new sccs\n");
#endif
}