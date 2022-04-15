int mpi_frontier(graph_t* graph, int start_vertex, int* result){
    int num_vertices = graph->num_vertices; 
    fill_n(result, num_vertices, MAX_DIST);

    auto start_time = Time::now();
    
    int depth = 0;
    result[start_vertex] = depth;

	/////
	int wr,ws;
	MPI_Comm_rank(MPI_COMM_WORLD, &wr);
	MPI_Comm_size(MPI_COMM_WORLD, &ws);
	MPI_Request reqs[ws];
	MPI_Status stats[ws];
	/////
    int *frontier_in = new int[num_vertices];
    int *frontier_out = new int[num_vertices];
    frontier_in[0] = start_vertex;
    int front_in_size = 1;
    int front_out_size = 0;
	int * incsize = new int[ws];
	int * disp = new int[ws];
    while (front_in_size != 0)
    {
        front_out_size = 0;

        for (int v = wr; v < front_in_size; v+=ws)
        {
            int vertex = frontier_in[v];

            for (int n = graph->v_adj_begin[vertex]; 
                n < graph->v_adj_begin[vertex] + graph->v_adj_length[vertex]; 
                n++)
            {
                int neighbor = graph->v_adj_list[n];

                if (result[neighbor] > depth+1)
                {
                    result[neighbor] = depth+1;
                    frontier_out[front_out_size] = neighbor;
                    front_out_size++;
                }
            }
        }
		MPI_Allgather(&front_out_size, 1, MPI_INT, incsize, 1, MPI_INT, MPI_COMM_WORLD);
		disp[0]=0;
		front_in_size=incsize[0];
		for(int ii=1;ii<ws;ii++){
			disp[ii]=incsize[ii-1]+disp[ii-1];
			front_in_size+=incsize[ii];
		}
		frontier_in = new int[front_in_size];
        MPI_Allgatherv(frontier_out, front_out_size, MPI_INT, frontier_in, incsize, disp, MPI_INT, MPI_COMM_WORLD);
		for(int ii=0;ii<front_in_size;ii++){
			int n=frontier_in[ii];
			if (result[n] > depth+1){
                result[n] = depth+1;
            }
		}
        depth++;
    }

    return std::chrono::duration_cast<us>(Time::now()-start_time).count();
}
