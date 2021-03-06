--more index select tests - make sure it can work with multiple indexes on a relation

CREATE TABLE USERS(userid integer, fname char(40), lname char(40), rating double);
CREATE TABLE USERS2(userid integer, fname char(40), lname char(40), rating double);

CREATE INDEX USERS(userid);
CREATE INDEX USERS2(rating);
CREATE INDEX USERS2(userid);
CREATE INDEX USERS(rating);

INSERT INTO USERS(userid, fname, lname, rating) VALUES(1, 'Trevor', 'Assaf', -5.0);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(1, '2Trevor', '2Assaf', 5.2);
INSERT INTO USERS(userid, fname, lname, rating) VALUES(2, 'Trevor2', 'Assaf2', 1.6);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(2, '2Trevor2', '2Assaf2', 3.6);

INSERT INTO USERS(userid, fname, lname, rating) VALUES(3, 'Trevor3', 'Assaf3', -15.2);
INSERT INTO USERS(userid, fname, lname, rating) VALUES(4, 'Trevor4', 'Assaf4', 9.3);

--See if it can handle duplicates
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(3, '2Trevor', '2Assaf', -1.3);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(4, '2Trevor4', '2Assaf4', 8.6);
INSERT INTO USERS2(userid, fname, lname, rating) VALUES(4, '2Trevor4', '2Assaf4', 8.6);

SELECT * FROM USERS WHERE USERS.userid <= 2;
SELECT * FROM USERS2 WHERE USERS2.rating = 8.6;
SELECT USERS2.lname, USERS2.fname FROM USERS2 WHERE USERS2.userid >= 1;
SELECT USERS.rating FROM USERS WHERE USERS.userid = 2;
SELECT USERS2.rating, USERS2.userid FROM USERS2 WHERE USERS2.rating < 0.4;
SELECT USERS.fname FROM USERS WHERE USERS.rating = 0.0;
SELECT USERS2.fname, USERS2.lname FROM USERS2 WHERE USERS2.userid <> 3;

SELECT USERS.fname FROM USERS WHERE USERS.fname = 'Hussein';

