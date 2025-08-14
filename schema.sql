-- Intelligent Course Scheduler schema and sample data (MySQL)
CREATE DATABASE IF NOT EXISTS course_scheduler;
USE course_scheduler;

CREATE TABLE IF NOT EXISTS courses (
  course_id INT PRIMARY KEY,
  name VARCHAR(80) NOT NULL
);

CREATE TABLE IF NOT EXISTS prerequisites (
  course_id INT NOT NULL,
  prereq_id INT NOT NULL,
  PRIMARY KEY (course_id, prereq_id),
  FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE,
  FOREIGN KEY (prereq_id) REFERENCES courses(course_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS timeslots (
  course_id INT NOT NULL,
  day_of_week ENUM('Mon','Tue','Wed','Thu','Fri') NOT NULL,
  start_time INT NOT NULL, -- e.g., 900 for 09:00
  end_time   INT NOT NULL, -- e.g., 1030 for 10:30
  PRIMARY KEY (course_id, day_of_week),
  FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE
);

-- Sample data
INSERT IGNORE INTO courses VALUES
(1,'Programming Fundamentals'),
(2,'Data Structures'),
(3,'Algorithms'),
(4,'Databases'),
(5,'Operating Systems');

INSERT IGNORE INTO prerequisites VALUES
(2,1), -- DS requires PF
(3,2), -- Algo requires DS
(5,2); -- OS requires DS

INSERT IGNORE INTO timeslots VALUES
(1,'Mon',900,1030),
(2,'Mon',1100,1230),
(3,'Wed',900,1030),
(4,'Tue',1400,1530),
(5,'Thu',1400,1530);
