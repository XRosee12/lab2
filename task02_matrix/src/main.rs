use std::io::{self, Write};

fn read_number(prompt: &str) -> usize {
    loop {
        print!("{}", prompt);
        io::stdout().flush().unwrap();

        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();

        if let Ok(num) = input.trim().parse::<usize>() {
            if num > 0 {
                return num;
            }
        }

        println!("1 이상의 정수를 입력하세요.");
    }
}

fn read_matrix(rows: usize, cols: usize, name: &str) -> Vec<Vec<i32>> {
    let mut matrix = Vec::with_capacity(rows);

    println!("\n{} 행렬을 입력하세요.", name);

    for i in 0..rows {
        loop {
            print!("{}행 (숫자 {}개): ", i + 1, cols);
            io::stdout().flush().unwrap();

            let mut input = String::new();
            io::stdin().read_line(&mut input).unwrap();

            let numbers: Result<Vec<i32>, _> = input
                .split_whitespace()
                .map(|s| s.parse::<i32>())
                .collect();

            match numbers {
                Ok(row) if row.len() == cols => {
                    matrix.push(row);
                    break;
                }
                _ => println!("정수 {}개를 공백으로 구분해 입력하세요.", cols),
            }
        }
    }

    matrix
}

fn main() {
    let rows = read_number("행의 개수: ");
    let cols = read_number("열의 개수: ");

    let a = read_matrix(rows, cols, "첫 번째");
    let b = read_matrix(rows, cols, "두 번째");

    let mut result = vec![vec![0i32; cols]; rows];

    for i in 0..rows {
        for j in 0..cols {
            result[i][j] = a[i][j] + b[i][j];
        }
    }

    println!("\n행렬 덧셈 결과:");

    for row in &result {
        for value in row {
            print!("{} ", value);
        }
        println!();
    }
}
