-- Базовая схема взята из предоставленного файла CarPark.sql.
-- Приложение использует эти же таблицы и поля:
-- Users, Drivers, Vehicles, Orders, Driver_earnings.
.read data/CarPark.sql

CREATE TRIGGER IF NOT EXISTS trg_orders_check_capacity
BEFORE INSERT ON Orders
FOR EACH ROW
BEGIN
    SELECT CASE
        WHEN NEW.cargo_weight > (SELECT capacity FROM Vehicles WHERE id = NEW.vehicle_id)
        THEN RAISE(ABORT, 'Cargo mass exceeds vehicle capacity')
    END;
END;
