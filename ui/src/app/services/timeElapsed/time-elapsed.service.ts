import {Injectable, NgZone} from '@angular/core';
import {Observable, interval, startWith} from 'rxjs';
import {map} from 'rxjs/operators';

@Injectable({
  providedIn: 'root'
})
export class TimeElapsedService {
  constructor(private ngZone: NgZone) {
  }

  getElapsedTime(startTime: number): Observable<number> {
    return this.ngZone.runOutsideAngular(() =>
      interval(1000).pipe(startWith(0),
        map(() => Math.floor((Date.now() - startTime) / 1000))
      )
    );
  }

  time() {
    return interval(1000).pipe(
      startWith(Date.now()),
      map(_ => Date.now())
    )
  }
}
