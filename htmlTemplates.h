#pragma once

#include <QString>

// %1 - task folder name
// %2 - time and date
QString reportHeader =
		"<section class=\"u-clearfix u-section-1\" id==\"sec-c7e3=\">"
		"<div class=\"u-clearfix u-sheet u-sheet-1\">"
		"<p class=\"u-align-left u-large-text u-text u-text-variant u-text-1\">"
		"<span style=\"font-weight: 700;\">"
		"%1<br>%2"
		"</span></p></div></section><section class=\"u-align-center u-clearfix u-section-2\" id=\"sec-82ab\">"
		"<div class=\"u-clearfix u-sheet u-sheet-1\">"
		"<div class=\"u-expanded-width u-table u-table-responsive u-table-1\">"
		"<table class=\"u-table-entity u-table-entity-1\">"
		"<colgroup><col width=\"25\%\"><col width=\"25\%\"><col width=\"25%\"><col width=\"25\%\"></colgroup>"
		"<tbody class=\"u-align-left u-custom-font u-font-montserrat u-table-alt-palette-1-light-3 u-table-body\">"
		"<tr style=\"height: 28px;\">"
		"<td class=\"u-custom-font u-first-column u-font-montserrat u-table-cell u-table-cell-1\">"
		"Имя<br></td><td class=\"u-table-cell\" spellcheck=\"false\">Тест</td>"
		"<td class=\"u-table-cell\" spellcheck=\"false\">Статус<br></td><td class=\"u-table-cell\" spellcheck=\"false\">"
		"Время исполнения<br></td></tr>";

QString greenCssClass = "u-custom-color-1 u-text-black";
QString yellowCssClass = "u-palette-3-light-1 u-text-black";
QString blackCssClass = "u-custom-color-2";

// %1 - color
// &2 - qrs
// %3 - field
// %4 - status
// %5 - execution time
QString taskReport =
		"<tr style=\"height: 58px;\">"
		"<td style=\"font-weight: bold\" class=\"u-custom-font u-first-column u-font-montserrat %1 u-table-cell\">%2</td>"
		"<td class=\"u-table-cell\">%3</td><td class=\"u-table-cell\">%4</td><td class=\"u-table-cell\">%5</td>"
		"</tr>";

