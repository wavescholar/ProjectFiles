function [ output_args ] = GrowClusteredGraph( Nodes,Clusters, Probs )

N=Nodes;

x = randperm(N);
gs = N/clusters;

%Make a struct with the cluster graphs G_C, and then mix to get  
for C=1:clusters
probs(i)
G.C = x(1:gs);
G2 = x(gs+1:end);
p_G1 = 0.45;
p_G2 = 0.45;
p_Inbetween = 0.1;
A(G1, G1) = rand(gs,gs) < p_G1;
A(G2, G2) = rand(N-gs,N-gs) < p_G2;
A(G1, G2) = rand(gs, N-gs) < p_Inbetween;
end

A = triu(A,1);
A = A + A';spy(A);
L = del2(A);
[V D] = eigs(L, 2, 'SA');
D(2,2);%ans = 46.7158
plot(V(:,2), '.-');
plot(sort(V(:,2)), '.-');
[ignore p] = sort(V(:,2));
spy(A(p,p));draw_dot(A(p,p));
%[ignore p] = sort(V(:,2));
%spy(A(p,p));
%Let's do an MDS on the graph adjacency
 [points,vaf]=mds(A,4);%metric,iterations,learnrate)
 %D=all_shortest_paths(A);
 plot(points(:,1),points(:,2));
[ w_mh   ] = mh(A);
plotgraphBruce(A,points,w_mh);
return A;
end
