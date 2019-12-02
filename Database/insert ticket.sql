DROP PROCEDURE IF EXISTS proc_initData;
DELIMITER $
CREATE PROCEDURE proc_initData()
	BEGIN
		DECLARE a INT DEFAULT 1;
        DEClARE b INT DEFAULT 1;
        SET a = 1;
		WHILE a<=20 DO
			SET b = 1;
			WHILE b<=100 DO
				INSERT INTO train.20191201_G102(status,start,end,seat_number,compartment_number) VALUES("unsold",1,11,b,a);
				SET b = b+1;
			END WHILE;
            SET a = a+1;
		END WHILE;
	END $
CALL proc_initData();