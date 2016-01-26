<p align="center">
    <a href="http://sphia.org"><img src="http://serenitydb.org/logo.png" width="20%" height="20%" /></a>
</p>
<p align="center">
    disk storage and real transactions under Redis compatible protocol
    <br>
    <a href="http://serenitydb.org/">serenity database</a>
    <br>
    <br>
</p>

<p><b>ABOUT</b>

<p>Serenity database implements basic Redis commands and extends them with
support of Consistent Cursors, ACID transactions,
Stored procedures, etc.

<p>At the same time being compatible with the Redis configuration, client drivers, command clients, benchmark tools, etc.
<p>Serenity is build on top of Append-Only MVCC storage engine <a href="http://sphia.org">Sophia</a>, which is designed to efficiently store data much larger then available RAM.
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
