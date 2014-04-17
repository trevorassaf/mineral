-- Insert: test order of attributes
CREATE TABLE USERS(userid integer, fname char(20), lname char(20), age integer);

INSERT INTO USERS(userid, lname, fname, age) VALUES(1, 'Assaf', 'Trevor', 21);
INSERT INTO USERS(userid, lname, age, fname) VALUES(2, 'Assaf', 21, 'Trevor');
INSERT INTO USERS(userid, age, lname, fname) VALUES(3, 21, 'Assaf', 'Trevor');
INSERT INTO USERS(lname, fname, age, userid) VALUES('Assaf', 'Trevor', 21, 4);

SELECT * FROM USERS;

SELECT * FROM USERS WHERE USERS.fname = 'Hussein'
