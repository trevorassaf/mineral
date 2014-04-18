--test joins with duplicates

CREATE TABLE USERS(userid integer, fname char(40), lname char(40), rating double);
CREATE TABLE USERS2(userid integer, fname char(40), lname char(40), rating double);

INSERT INTO USERS(userid, fname, lname, rating) VALUES(1, 'Trevor', 'Assaf', -5.0);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(1, 'Trevor', '2Assaf', 5.2);
INSERT INTO USERS(userid, fname, lname, rating) VALUES(2, 'Trevor', 'Assaf2', 1.6);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(2, 'Trevor', '2Assaf2', 3.6);

INSERT INTO USERS(userid, fname, lname, rating) VALUES(3, 'Trevor', 'Assaf3', -15.2);
INSERT INTO USERS(userid, fname, lname, rating) VALUES(4, 'Trevor', 'Assaf4', 9.3);

INSERT INTO USERS2(userid, fname, lname, rating) VALUES(3, 'Trevor', '2Assaf3', -1.3);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(4, 'Trevor', '2Assaf4', 8.6);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(4, 'Trevor', '2Assaf4', 8.6);

--smj joins
SELECT USERS.userid, USERS2.userid FROM USERS, USERS2 WHERE USERS.fname = USERS2.fname;
SELECT * FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;
--index joins, same query with different indexes present
CREATE INDEX USERS(userid);
SELECT * FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;
DROP INDEX USERS(userid);
CREATE INDEX USERS2(userid);
SELECT USERS.lname, USERS2.fname, USERS2.lname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;
CREATE INDEX USERS(userid);
SELECT USERS.fname, USERS2.fname, USERS.lname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;

--snl join
SELECT * FROM USERS, USERS2 WHERE USERS.userid < USERS2.userid;

INSERT INTO USERS(userid, fname, lname, rating) VALUES(5, 'Trevor2', 'Assaf5', 3.5);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(5, 'Trevor2', '2Assaf5', 6.0);
INSERT INTO USERS(userid, fname, lname, rating) VALUES(6, 'Trevor2', 'Assaf6', 7.1);
SELECT USERS.userid, USERS2.userid FROM USERS, USERS2 WHERE USERS.fname = USERS2.fname;

SELECT * FROM USERS WHERE USERS.fname = 'Hussein';

