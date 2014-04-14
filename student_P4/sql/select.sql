-- Test selection queries

SELECT * FROM DA WHERE DA.serial = 11; -- simple filescan

CREATE INDEX DA (serial);
SELECT * FROM DA WHERE DA.serial = 11;  -- use index select

DROP INDEX DA (serial);
SELECT * FROM DA WHERE DA.serial = 11; -- simple filescan

-- more filescan queries
SELECT * FROM DA WHERE DA.dkey > 100.0;

SELECT * FROM DA WHERE DA.dkey >= 106.7;

SELECT * FROM DA WHERE DA.filler = '00062 string record';
