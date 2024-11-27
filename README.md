# Histograma

Este é o repositório que contém o trabalho da disciplina Introdução à Programação Paralela e Distribuída.
Nele, foi feita a paralelização de um problema de histograma, onde o input é um arquivo de texto de uma imagem PPM e o output é uma matriz com a frequência de cada cor.
O problema foi retirado da Marathon of Parallel Programming, sendo que o código fonte sequencial utilizado neste trabalho pode ser encontrado [no site da maratona](http://lspd.mackenzie.br/marathon/16/problems.html).

## Como foi feita a paralelização?

### 1. Normalização dos valores dos pixels

```C
#pragma omp parallel for
for (i = 0; i < n; i++) {
  image->data[i].red = floor((image->data[i].red * DIVS) / 256.0);
  image->data[i].green = floor((image->data[i].green * DIVS) / 256.0);
  image->data[i].blue = floor((image->data[i].blue * DIVS) / 256.0);
}
```

Essa parte paraleliza o processo de normalização dos pixels, ou seja, cada thread processa uma parte dos pixels de forma independente.

### 2. Inicialização do histograma

```C
#pragma omp parallel for
for (i = 0; i < DIVS * DIVS * DIVS; i++) {
  h[i] = 0.0;
}
```

Aqui também foi feito o mesmo que anteriormente para paralelizar a inicialização do histograma.

### 3. Cálculo do histograma

```C
#pragma omp parallel
{
  float local_h[DIVS * DIVS * DIVS] = {0};

  #pragma omp for collapse(3)
  for (j = 0; j < DIVS; j++) {
    for (k = 0; k < DIVS; k++) {
      for (l = 0; l < DIVS; l++) {
        int count = 0;
        for (i = 0; i < n; i++) {
          if (image->data[i].red == j && image->data[i].green == k &&
              image->data[i].blue == l) {
            count++;
          }
        }
        local_h[j * DIVS * DIVS + k * DIVS + l] = count;
      }
    }
  }

  #pragma omp critical
  {
    for (int idx = 0; idx < DIVS * DIVS * DIVS; idx++) {
      h[idx] += local_h[idx];
    }
  }
}
```

Essa é a parte mais complexa do código. Após criar a região paralela, foi feito o uso `local_h` para criar um vetor local e armazenar os resultados parciais do programa e evitar condições de corrida.
Depois disso, é feito o uso de `#pragma omp for collapse(3)`, o que faz com que o loop de `j`, `k` e `l` sejam paralelizados combinando as três iterações em uma única iteração paralelizada.
Por fim, depois que as threads calculam os valores locais em `local_h`, os resultados são combinados no vetor `h`. Para isso, foi utilizado `#pragma omp critical` que garante que o acesso à `h` seja feito de forma segura, ou seja, apenas uma thread por vez pode atualizar o valor de `h`.
