
<p>Serenity database implements basic Redis commands and extends them with
support of Consistent Cursors, ACID transactions,
Stored procedures, etc.

<p>This project is a proof-of-concept.

<p>At the same time being compatible with the Redis configuration, client drivers, command clients, benchmark tools, etc.
<p>Serenity is build on top of Append-Only MVCC storage engine <a href="http://sphia.org">Sophia</a>, which is designed to efficiently store
data much larger then available RAM.
<br>
<br>
<p><b>BUILD AND USE</b>

<p>git clone --recursive https://github.com/pmwkaa/serenity.git
<br>
make<br><br>

<p>Edit <b>serenity.conf</b> file or run <b>serenity</b> binary as is to start the database in default configuration.<br>
Use redis-cli, redis-benchmark, memtier_benchmark to test.

<p><b>Docker</b>

<pre>docker run --rm -it -p 6379:6379 fgribreau/serenity</pre>
</p>
