-- scan select test: make sure basic selects work

CREATE TABLE USERS(userid integer, fname char(40), lname char(40));
CREATE TABLE USERS2(userid integer, fname char(40), lname char(40));

INSERT INTO USERS(userid, fname, lname) VALUES(1, 'Trevor', 'Assaf');
INSERT INTO USERS2(userid, fname, lname) VALUES(1, '2Trevor', '2Assaf');
INSERT INTO USERS(userid, fname, lname) VALUES(2, 'Trevor2', 'Assaf2');
INSERT INTO USERS2(userid, fname, lname) VALUES(2, '2Trevor2', '2Assaf2');

INSERT INTO USERS(userid, fname, lname) VALUES(3, 'Trevor3', 'Assaf3');
INSERT INTO USERS(userid, fname, lname) VALUES(4, 'Trevor4', 'Assaf4');

INSERT INTO USERS2(userid, fname, lname) VALUES(3, '2Trevor', '2Assaf');
INSERT INTO USERS2(userid, fname, lname) VALUES(4, '2Trevor4', '2Assaf4');

SELECT * FROM USERS WHERE USERS.userid <= 2;
SELECT * FROM USERS2 WHERE USERS2.fname = '2Trevor2';
SELECT USERS2.fname FROM USERS2 WHERE USERS2.userid > 1;
SELECT USERS.lname, USERS.userid FROM USERS WHERE USERS.fname = 'Trevor';
--Selection where duplicate values will be produced
SELECT USERS2.fname, USERS2.lname FROM USERS2 WHERE USERS2.userid <> 4;

-- index select test: make sure basic selects work

CREATE INDEX USERS(userid);
CREATE INDEX USERS2(userid);
CREATE INDEX USERS2(fname);


SELECT * FROM USERS WHERE USERS.userid <= 2;
SELECT * FROM USERS WHERE USERS.userid = 2;
SELECT * FROM USERS2 WHERE USERS2.fname = '2Trevor2';
SELECT USERS2.fname FROM USERS2 WHERE USERS2.userid > 1;
SELECT USERS.lname, USERS.userid FROM USERS WHERE USERS.fname = 'Trevor';
--Selection where duplicate values will be produced
SELECT USERS2.fname, USERS2.lname FROM USERS2 WHERE USERS2.userid <> 4;

SELECT USERS.fname FROM USERS WHERE USERS.fname = 'Hussein';
