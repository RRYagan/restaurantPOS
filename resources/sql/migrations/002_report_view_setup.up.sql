
CREATE VIEW vw_sales_report AS
SELECT 
    co.id AS order_id,
    co.waiter_id,
    co.order_status,
    DATE(COALESCE(s.sale_transaction_date, co.created_at)) AS sale_date,
    STRFTIME('%H', COALESCE(s.sale_transaction_date, co.created_at)) AS sale_hour,
    COALESCE(s.gross_amount, 0.0) AS gross_amount,
    COALESCE(s.kra_receipt_number, 'MISSING') AS kra_receipt_number,
    
    -- Payment Details
    COALESCE(pm.payment_method_code_name, 'PENDING') AS payment_method,
    -- Removed p.name reference; using id and external_ref only
    COALESCE(p.external_reference, CAST(p.id AS TEXT), '') AS payment_id

FROM customer_order co
LEFT JOIN sale s ON s.order_id = co.id
LEFT JOIN payment_method pm ON s.payment_method_id = pm.id
-- Ensure the table name is 'payments' (plural) and not 'payment'
LEFT JOIN payments p ON p.order_id = co.id AND p.status = 'Success';
