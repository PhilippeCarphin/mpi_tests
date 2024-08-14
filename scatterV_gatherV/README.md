# Experiment with `MPI_Scatterv,MPI_Gatherv`

## Program rundown

This test program demonstrates the use of the `v` versions of `MPI_Gather` and
`MPI_Scatter` for when tiles are not of uniform sizes.

```
coupler --(MPI_ScatterV)--> model --(add local_rank+1 to values)--> model --(MPI_Gatherv)--> coupler
```

```
int MPI_Gatherv(
    // Sender stuff
    const void *sendbuf, int sendcnt, MPI_Datatype sendtype,
    // Receiver stuff
    void *recvbuf, const int *recvcnts, const int *displs, MPI_Datatype recvtype,
    int root,
    MPI_Comm comm
)
```

and `MPI_Scatterv()` is similar.

## The `root` argument

See: [When comm is an inter-communocator](https://www.open-mpi.org/doc/v4.0/man3/MPI_Scatterv.3.php#toc9)
The manpage on the system doesn't have that section.

My understanding of this for `MPI_Scatterv` is that there are two groups, one
group is the sender and one group is the receiver.  A single process form the
sender group sends data to all processes of the receiver group.

The sender group is specified by setting the `root` argument to either `MPI_ROOT`
or `MPI_PROC_NULL`.  The process with `MPI_ROOT` is the one that does the sending and there should only be one of them.
The other processes with `MPI_PROC_NULL` as their root will do nothing (in general,
giving `MPI_PROC_NULL` in an MPI call where a rank is expected turns that call
into a noop so

```
MPI_Scatterv(
    ...,
    (local_rank == x ? MPI_ROOT : MPI_PROC_NULL),
    ...
)
```
is like doing
```
if(local_rank == x){
    MPI_Scatterv(..., MPI_ROOT, ...)
}
