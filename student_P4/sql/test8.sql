--Join on no matches, matches at the end, and then duplicate matches at the end (mostly for smj)

CREATE TABLE USERS(userid integer, fname char(20), lname char(20));
CREATE TABLE USERS2(userid integer, fname char(20), lname char(20));

INSERT INTO USERS(userid, fname, lname) VALUES(1, 'Trevor', 'Assaf');
INSERT INTO USERS2(userid, fname, lname) VALUES(2, '2Trevor2', '2Assaf2');
INSERT INTO USERS(userid, fname, lname) VALUES(3, 'Trevor3', 'Assaf3');
INSERT INTO USERS2(userid, fname, lname) VALUES(4, '2Trevor4', '2Assaf4');
INSERT INTO USERS(userid, fname, lname) VALUES(5, 'Trevor5', 'Assaf5');
INSERT INTO USERS2(userid, fname, lname) VALUES(6, '2Trevor6', '2Assaf6');
INSERT INTO USERS(userid, fname, lname) VALUES(7, 'Trevor7', 'Assaf7');
INSERT INTO USERS2(userid, fname, lname) VALUES(8, '2Trevor8', '2Assaf8');

SELECT USERS.lname, USERS2.fname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;

SELECT USERS.fname, USERS2.lname FROM USERS, USERS2 WHERE USERS.userid <> USERS2.userid;

INSERT INTO USERS(userid, fname, lname) VALUES(9, 'Trevor9', 'Assaf9');
INSERT INTO USERS2(userid, fname, lname) VALUES(9, '2Trevor9', '2Assaf9');

SELECT USERS.lname, USERS2.fname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;

INSERT INTO USERS(userid, fname, lname) VALUES(9, 'Trevor9', 'Assaf9');
INSERT INTO USERS2(userid, fname, lname) VALUES(9, '2Trevor9', '2Assaf9');
INSERT INTO USERS(userid, fname, lname) VALUES(10, 'Trevor10', 'Assaf10');
INSERT INTO USERS2(userid, fname, lname) VALUES(10, '2Trevor10', '2Assaf10');

SELECT USERS.lname, USERS2.fname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;

INSERT INTO USERS(userid, fname, lname) VALUES(10, 'Trevor10', 'Assaf10');
INSERT INTO USERS2(userid, fname, lname) VALUES(10, '2Trevor10', '2Assaf10');

SELECT USERS.lname, USERS2.fname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;

CREATE INDEX USERS2(userid); 
SELECT USERS.lname, USERS2.lname FROM USERS, USERS2 WHERE USERS.userid = USERS2.userid;
DROP INDEX USERS2(userid);

SELECT * FROM USERS WHERE USERS.fname = 'Hussein';
