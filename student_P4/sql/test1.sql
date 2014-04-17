-- Error handling

CREATE TABLE USERS(userid integer, fname char(20), lname char(20), age integer);

INSERT INTO USERS(userid, fname, lname) VALUES(1, 'Trevor', 'Assaf');

INSERT INTO USERS(userid, fname, lname, age, gender) 
  VALUES(1, 'Trevor', 'Assaf', 21, 0);

INSERT INTO USERS(userid, fname, lname, gender) VALUES(1, 'Trevor', 'Assaf', 0);

SELECT * FROM USERS WHERE USERS.fname = 'Hussein';
