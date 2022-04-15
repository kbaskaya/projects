int mpi_vertex_dist(graph_t *graph, int start_vertex, int *result)
{
    int num_vertices = graph->num_vertices;
    fill_n(result, num_vertices, MAX_DIST);

    auto start_time = Time::now();

    int depth = 0;
    result[start_vertex] = depth;
    int keep_going = 1;
    /////////
    int wr, ws;
    MPI_Comm_rank(MPI_COMM_WORLD, &wr);
    MPI_Comm_size(MPI_COMM_WORLD, &ws);
    MPI_Request reqs[ws];
    MPI_Status stats[ws];
    ////////
    while (keep_going)
    {
        keep_going = 0;

        for (int vertex = wr; vertex < num_vertices; vertex += ws)
        {
            if (result[vertex] == depth)
            {
                for (int n = graph->v_adj_begin[vertex];
                     n < graph->v_adj_begin[vertex] + graph->v_adj_length[vertex];
                     n++)
                {
                    int neighbor = graph->v_adj_list[n];

                    if (result[neighbor] > depth + 1)
                    {
                        result[neighbor] = depth + 1;
                        keep_going = 1;
                        for (int other = 0; other < ws; other++)
                        {
                            if (other != wr)
                                MPI_Send(&neighbor, 1, MPI_INT, other, 1, MPI_COMM_WORLD);
                        }
                    }
                }
            }
        }
        for (int other = 0; other < ws; other++)
        {
            if (other != wr) // 2 is stop tag
                MPI_Send(&keep_going, 1, MPI_INT, other, 2, MPI_COMM_WORLD);
        }
        int stop_count = 0;
        while (stop_count < ws - 1)
        {
            int inmsg = -1;
            MPI_Recv(&inmsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stats[wr]);
            if (stats[wr].MPI_TAG == 2)
            {
                stop_count++;
                keep_going += inmsg;
            }
            else
            {
                result[inmsg] = depth + 1;
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        depth++;
    }

    // print_result(graph, result, depth);
    return std::chrono::duration_cast<us>(Time::now() - start_time).count();
}
