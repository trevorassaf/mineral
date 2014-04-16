-- Test selection queries

SELECT * FROM DB WHERE DB.serial = 65;

--CREATE INDEX DA (ikey);

SELECT * FROM DA, DB WHERE DA.ikey = DB.ikey; -- use INL

--DROP INDEX DA (ikey);
SELECT * FROM DA, DB WHERE DA.ikey = DB.ikey; -- use SMJ

SELECT DA.ikey, DB.serial FROM DA, DB WHERE DA.ikey < DB.serial; -- use SNL

