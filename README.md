# HPSC Lab 5
2019-09-27

Click to [make your own repo](https://classroom.github.com/a/H0xr56JU).

The goals for this lab are:
* Add parallelism to a graph algorithm.
* Implement a component of homework 2.

WEEKLY WORDS OF WISDOM:
You may find that the majority of the work in HPC is dealing with other people's lousy code.  Sorry!

-----

The repository code `coloring.c` contains a (serial) implementation of an algorithm to find a [Maximal Independent Set](https://en.wikipedia.org/wiki/Maximal_independent_set) (MIS) of a graph.  As described in the September 23 lecture, this is a greedy algorithm that iteratively picks a vertex from the set of remaining candidates and then removes it and its neighbors from the candidate set, repeating such passes until the candidate set is empty.  This MIS is stuck inside of an outer loop, such that after an initial MIS is found, additional MIS are found among the remaining vertices.  In this way, every vertex is eventually given a color which is different than its neighbors.

In preparation for your parallelization, this implementation stores a graph as an adjacency list `neighbors`.  Each "row" in the list (broken up by the `offsets` variable) has an unordered list of the neighbors of the vertex given by the row index.  This implementation is also structured to create a compact list of candidates each iteration for slightly more efficient iterations (however, the `compact` function is initially linear).

Reflect on the implication that the only criteria for including an edge in the independent set is that none of its neighbors are included.  This means that we could simultaneously add as many edges as we'd like to the set if is no danger of two of them being neighbors.  When evaluating vertices independently, one can leverage their neighbor information to establish selection criteria that ensures that no vertex will be selected if one of its neighbors was selected.  The simplest option might be to only select a candidate if its neighboring candidates have higher index.  However, this is prone to inefficiencies due to ordering patterns when the data was originally created.  To mitigate such inefficiencies, we can permute the vertex order by ordering based on a hash of the index or permutation of the set, done in the `verthash` function.

To get a practice graph, check out CU's [Index of Complex Networks](https://icon.colorado.edu).  The `graphload` function provided by the lab is meant to operate on edge list files (lines of two vertex indices separated by a tab).  Try starting with [roadNet-PA](http://snap.stanford.edu/data/roadNet-PA.html), which has just over a million vertices.  Despite the comment at the top of the file, this set does contain both directions of the undirected edges, so leave the command line option `-r` as the default non-zero.

The `coloring` executable prints out the number of MIS iterations (colors) it needed to perform.  The infamous four color conjecture proved that all planar graphs could be split into four independent sets.  Road networks are close to planar meshes, although they can potentially contain overpasses and underpasses.  The greedy nature of MIS implementations, as well as the greedy nature of the implemented coloring, may also explain extra colors being used.

-----

Alter the `colorverts` outer loop to use multiple threads.  Where should thread synchronization be added?  You will likely need to add thread-specific copies of some variables, either implicitly with `omp task` or explicitly if using `omp parallel for`.

The implementation as otherwise given relies on in-order execution when updating `colorset` and `dropset` after a vertex has been determined to be included in the independent set.  That is, if `dropset` is flagged first, other neighbors may skip over that vertex when performing their calculations of the minimal `verthash` but then also check that vertex's `colorset` value before it was updated; this would look identical to the vertex having been dropped because it was a neighbor of an included vertex, rather than being an included vertex itself.  Not only can the compiler reorder the store instructions, but the hardware might complete them out of order.  To avoid these issues, the `colorset` and `dropset` memory are marked as [`volatile`](https://en.cppreference.com/w/c/language/volatile) in this function.  (In general, when the code blocks are not accessing volatile memory, something like [these Google benchmark functions](https://github.com/google/benchmark/blob/master/README.md#preventing-optimization) may be useful.)

Alter the `compact` loop to use multiple threads.  Hint: think back to the lecture on reduction/scan.  This should also be useful for part 1 of homework 2.  There is also another spot in the coloring loop where `dropset` is reset from `colorset` which could use some multithreading, which is also marked with `LINEAR VERSON`.

-----

Looking for more to do?  The implementation is not currently particularly smart about removing vertices from neighbors that have been dropped/colored so that it doesn't need to recheck them.  Make it smarter.  Note: don't alter the original `offsets` and `neighbors` arrays, or the validation won't work.