require 'rubyXL'
require "json"

class SpreadsheetParser
  def parse_workbook(filepath)
    workbook = RubyXL::Parser.parse(filepath)

    workbook.worksheets.map { |sheet| parse_sheet(sheet) }
  end

  def parse_sheet(sheet)
    entities = []

    current_entity = []

    prev_row_idx = nil

    sheet.each do |row|
      next unless row
      row_elems = []
      row_idx = row.r

      row && row.cells.map do |cell|
        val = cell && cell.value

        if val
          row_elems[cell.column] = {
            value: cell.value,
            color: cell.fill_color,
            bolded: !!cell.is_bolded,
            formula: cell.formula && cell.formula.expression.length > 0 && cell.formula.expression,
            blah: cell.formula
          }
        end
      end

      if prev_row_idx && row_idx - prev_row_idx != 1
        entities << current_entity if current_entity != []
        current_entity = [row_elems]
      else
        current_entity << row_elems if row_elems != []
      end

      prev_row_idx = row_idx if row_elems.length > 0
    end

    entities << current_entity if current_entity != []

    {
      name: sheet.sheet_name,
      entities: entities.map { |e| process_entity(e) }
    }
  end

  def process_entity(e)
    p e
    if e.length == 1 && e.first.length == 1
      if e[0][0][:bolded]
        {type: "h3", value: e[0][0][:value]}
      else
        {type: "p", value: e[0][0][:value]}
      end
    else
      if e[0].length == 1
        {
          type: "table",
          title: e[0][0][:value],
          table: process_table(e.drop(1))
        }
      else
        {
          type: "table",
          table: process_table(e)
        }
      end
    end
  end

  def process_table(e)
    height = e.length
    width = e.map(&:length).max

    res = e.map.with_index do |row, row_index|
      row.map do |cell|
        if cell.nil?
          nil
        elsif cell[:formula]
          {type: "formula", formula: cell[:formula], value: cell[:value]}
        elsif cell[:color] == "FFFFFF00"
          {type: "input", name: cell[:value], value: cell[:value]}
        elsif cell[:color] == "FFFFC000"
          {type: "result", name: cell[:value], value: cell[:value]}
        else
          if row_index == 0
            {type: "th", value: cell[:value]}
          else
            {type: "td", value: cell[:value]}
          end
        end
      end
    end
  end
end

if __FILE__ == $0
  puts SpreadsheetParser.new.parse_workbook("example_workbook.xlsx").to_json
end

