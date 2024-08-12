import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'in'
})
export class InPipe implements PipeTransform {

  transform<T>(value: T, arr: T[]): unknown {
    return arr.includes(value);
  }
}
