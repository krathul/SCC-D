#include "scc_delete_new_scc.cpp"


void fw_propagate(int rep_node, graph& g, int* reachable, int* fw_reach, del_set& deleted_edges){
#if DEBUG
	printf("Starting with the forwards marking from %d\n", rep_node);
#endif
	int temp_size = g.count_in_sccs[find_index(rep_node, g.rep_nodes, g.scc_count)];
	int* queue = new int[temp_size]; //assuming scc_counts will give the number of verts
	int* next_queue = new int[temp_size];
	int q_size=0, nxt_q_size=0;
	queue[0] = rep_node;
	q_size++;
	while(q_size){
		for(int i=0; i<q_size; i++){
			int vert = queue[i];
			//reachable[vert] = 1;
			if(g.scc_map[vert]==g.scc_map[rep_node]){ // ensure only scc rep can change fw node
				fw_reach[vert] = rep_node;
			}
			int Odeg = out_degree(g, vert); 
			int* Overts = out_vertices(g, vert);
			for(int j=0; j<Odeg; j++){
				int nVert = Overts[j];
				if((-1==fw_reach[nVert]) and (g.scc_map[nVert]==g.scc_map[rep_node]) and not_deleted(vert, nVert, deleted_edges)){ //checking for -1 as only rep_node is propagating hence other nodes cant come here
					next_queue[nxt_q_size++] = nVert;
				}
			}
		}
		q_size = nxt_q_size;
		swap(queue, next_queue);
		nxt_q_size = 0;
	}
	delete [] queue;
	delete [] next_queue;
#if DEBUG
	printf("Successfull marked all nodes reachable from %d\n", rep_node);
#endif
}

void bw_propagate(int rep_node, graph& g, int* reachable, int* fw_arr, int& unaffected, del_set& deleted_edges){
	int* scc_counts = g.count_in_sccs;
	int* scc_map = g.scc_map;
	int temp_size = scc_counts[find_index(scc_map[rep_node], g.rep_nodes, g.scc_count)];
	int* queue = new int[temp_size]; //assuming scc_counts will give the number of verts
	int* next_queue = new int[temp_size];
	int q_size=0, nxt_q_size=0;
	queue[0] = rep_node;
	q_size++;

	while(q_size){
		for(int i=0; i<q_size; i++){
			unaffected++;
			int vert = queue[i];
			reachable[vert] = 1;
			int Ideg = in_degree(g, vert);
			int* Iverts = in_vertices(g, vert);
			for(int j=0; j<Ideg; j++){
				int nVert = Iverts[j];
				if(scc_map[rep_node]==scc_map[nVert] and rep_node==fw_arr[nVert] and not_deleted(nVert, vert, deleted_edges) and !reachable[nVert]){
					next_queue[nxt_q_size++] = nVert;
#if DEBUG
					printf("Marking %d as reachable as it still belongs to %d scc\n",nVert,rep_node);
#endif
				}
			}
		}
		swap(queue, next_queue);
		q_size = nxt_q_size;
		nxt_q_size=0;
	}

	delete [] queue;
	delete [] next_queue;
}

void search(int rep_node, graph& g, int* reachable, int* fw_arr, int& unaffected, del_set& deleted_edges){
#if DEBUG
	printf("Starting to search %d \n", rep_node);
#endif
	fw_propagate(rep_node, g, reachable, fw_arr, deleted_edges);
	bw_propagate(rep_node, g, reachable, fw_arr, unaffected, deleted_edges);
#if DEBUG
	printf("finished search \n");
#endif
}

void naive_delete(del_set& deleted_edges, graph& g, int* out_q, int* in_q, int q_size){//for now not using * _q arrays
//	int* out_processed = new int[g.n];
//	int* in_processed = new int[g.n];
#if DEBUG
	printf("Start naive\n");
#endif
	int* reachable = new int[g.n];// to check if they are still part of scc
	int* fw_reach = new int[g.n]; //useful to take into account all vertices that can be reached from rep_node
				      
#if DEBUG
	printf("reachable = %p\n",reachable);
	printf("fw_reach = %p\n",fw_reach);
#endif
	int unaffected=0; //takes count of unaffected nodes
	for(int i=0; i<g.n; i++){
		fw_reach[i] = -1;
	}
	for(int i=0; i<g.n; i++){
		reachable[i]=0;
	}
	for(int i=0; i<g.scc_count; i++){ //assuming scc_no is available, can be calculated earier
		int vert = g.rep_nodes[i]; //assuming scc_array has like the list of rep_nodes
		if(!reachable[vert]){
			search(vert, g, reachable, fw_reach, unaffected, deleted_edges);
		}
	}
	
	int* unreachable = new int[g.n-unaffected];//array to keep list of verts that needs to be changed
	int affected = 0;
	for(int i=0; i<g.n; i++){
		if(!reachable[i]){
			unreachable[affected++] = i;
		}
	}
	
	create_new_sccs(unreachable, reachable ,affected, g, fw_reach, deleted_edges); //unreachable is array filled with changed verts (bad var names)
#if DEBUG
	printf("reachable = %p\n",reachable);
	printf("fw_reach = %p\n",fw_reach);
#endif
	delete [] fw_reach;
#if DEBUG
	printf("Finished naive\n");
#endif

	delete [] reachable;
}