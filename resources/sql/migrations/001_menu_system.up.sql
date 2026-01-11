BEGIN TRANSACTION;

-- -----------------------------
-- MENU CATEGORIES
-- -----------------------------
CREATE TABLE IF NOT EXISTS menu_categories (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL UNIQUE
);

-- -----------------------------
-- MENU ITEMS (Sellable items)
-- -----------------------------
CREATE TABLE IF NOT EXISTS menu_items (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    category_id TEXT,
    base_price_cents INTEGER NOT NULL,
    tax_ty_cd TEXT DEFAULT 'B',
    is_available BOOLEAN DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES menu_categories(id)
);

-- -----------------------------
-- MENU ITEM ↔ PRODUCT (Recipe)
-- -----------------------------
CREATE TABLE IF NOT EXISTS menu_item_products (
    menu_item_id TEXT NOT NULL,
    product_id TEXT NOT NULL,
    quantity REAL NOT NULL,
    PRIMARY KEY (menu_item_id, product_id),
    FOREIGN KEY (menu_item_id) REFERENCES menu_items(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id)
);

-- -----------------------------
-- UPDATE ORDER ITEMS
-- -----------------------------
-- Orders should reference menu_items, not products
ALTER TABLE order_items RENAME COLUMN product_id TO menu_item_id;

COMMIT;
