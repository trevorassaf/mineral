--
-- test catalogs
--

SELECT * FROM relcat;
SELECT * FROM attrcat;

-- create relations 
CREATE TABLE soaps(soapid integer, name char(32), network char(4), rating double);
CREATE INDEX soaps (soapid);

CREATE TABLE stars(starid integer, real_name char(20), plays char(12), soapid integer);

SELECT relcat.relname, relcat.attrcnt, relcat.indexcnt 
FROM relcat;

SELECT * FROM relcat;

SELECT attrcat.relname, attrcat.attrname, attrcat.attroffset, attrcat.attrtype,
	attrcat.attrlen, attrcat.indexed
FROM attrcat;

DROP TABLE soaps;

SELECT relcat.relname, relcat.attrcnt, relcat.indexcnt 
FROM relcat;

SELECT attrcat.relname, attrcat.attrname, attrcat.attroffset, attrcat.attrtype,
	attrcat.attrlen, attrcat.indexed
FROM attrcat;

DROP TABLE stars;

