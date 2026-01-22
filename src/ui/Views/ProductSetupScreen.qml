import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    color: "transparent"
Button {
    text: "Add Sample Product"
    onClicked: {
        var newProduct = {
            kraUniqueItemCode: "FIN-PIZZA-01",
            internalName: "Pizza Margherita",
            categoryCode: "2",
            sellingPrice: 800,
            taxClassificationId: 1,
            measurementUnitId: 1
        };
        controller.addProduct(newProduct);
    }
}

Button {
    text: "Add Ingredient"
    onClicked: {
        var ingredient = {
            productId: 1,               // Pizza
            ingredientProductId: 2,     // Flour
            quantity: 0.3,
            measurementUnitId: 1
        };
        controller.addIngredient(ingredient);
    }
}
}
