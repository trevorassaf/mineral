-- Insert: test insert into multiple relations

CREATE TABLE USERS(userid integer, fname char(20), lname char(20));
CREATE TABLE USERS2(userid integer, fname char(20), lname char(20));

INSERT INTO USERS(userid, fname, lname) VALUES(1, 'Trevor', 'Assaf');
INSERT INTO USERS2(userid, fname, lname) VALUES(1, '2Trevor', '2Assaf');
INSERT INTO USERS(userid, fname, lname) VALUES(2, 'Trevor2', 'Assaf2');
INSERT INTO USERS2(userid, fname, lname) VALUES(2, '2Trevor2', '2Assaf2');

INSERT INTO USERS(userid, fname, lname) VALUES(3, 'Trevor3', 'Assaf3');
INSERT INTO USERS(userid, fname, lname) VALUES(4, 'Trevor4', 'Assaf4');

INSERT INTO USERS2(userid, fname, lname) VALUES(3, '2Trevor3', '2Assaf3');
INSERT INTO USERS2(userid, fname, lname) VALUES(4, '2Trevor4', '2Assaf4');

SELECT * FROM USERS;
SELECT * FROM USERS2;

SELECT * FROM USERS WHERE USERS.fname = 'Hussein';
